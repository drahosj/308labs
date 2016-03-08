/**
 * @file      main.c
 * @author    Jeramie Vens
 * @date      2015-02-11: Created
 * @date      2015-02-15: Last updated
 * @date      2015-02-16: Complete re-write
 * @brief     Emulate a print server system
 * @copyright MIT License (c) 2015, 2016
 */
 
/*
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "print_job.h"
#include "printer_driver.h"
#include "debug.h"
#include "printserver.h"

#define ACCEPT_WELCOME "Connected to print server. Enter commands\n"

// -- GLOBAL VARIABLES -- //
int verbose_flag = 0;
int exit_flag = 0;
FILE * logfile = NULL;

// -- STATIC VARIABLES -- //
static struct printer_group * printer_group_head;
static int num_printers = 0;

/**
 * A list of print jobs that must be kept thread safe
 */
struct print_job_list
{
	// the head of the list
	struct print_job * head;
	// the number of jobs in the list
	sem_t num_jobs;
	// a lock for the list
	pthread_mutex_t lock;
};

/**
 * A printer object with associated thread
 */
struct printer
{
	// the next printer in the group
	struct printer * next;
	// the driver for this printer
	struct printer_driver driver;
	// the list of jobs this printer can pull from
	struct print_job_list * job_queue;
	// the thread id for this printer thread
	pthread_t tid;
};

/**
 * A printer group.  A group represents a collection of printers that can pull
 * from the same job queue.
 */
struct printer_group
{
	// the next group in the system
	struct printer_group * next_group;
	// the name of this group
	char * name;
	// the list of printers in this group
	struct printer * printer_queue;
	// the list of jobs for this group
	struct print_job_list job_queue;
};

// -- FUNCTION PROTOTYPES -- //
static void parse_command_line(int argc, char * argv[]);
static void parse_rc_file(FILE* fp);
static struct print_job * get_job(struct print_job_list * list);
static void put_job(struct print_job_list * list, struct print_job * job);
static void catch_error(int error);
static void handle_job(struct printer * this);
static void send_list(FILE * conn);

/** Tell all printers to exit */
static int kill_flag = 0;
static pthread_mutex_t kill_lock = PTHREAD_MUTEX_INITIALIZER;


void *printer_thread(void* param)
{
	struct printer * this = param;

	printf("Thread started for %s\n", this->driver.name);
	while(1)
	{
		/* Basic flow:
		 *
		 * - Handle as many jobs as are currently waiting
		 * - Once all pending jobs are handled, check the kill flag. If the 
		 *   kill flag is set, terminate the thread.
		 * - Go into a one second sleep. If a job appears during this sleep,
		 *   wake up and handle it immediately.
		 */


		/* Lock the kill flag, do an instantaneous check for
		 * pending jobs, then check the kill flag.
		 *
		 * By locking it, we know we won't miss out on any pending jobs
		 * because the kill flag can't change between checking for jobs
		 * and checking it.
		 */

		int job_pending = 0;
		do {
			catch_error(pthread_mutex_lock(&kill_lock));

			/* Determine if a job is pending */
			if (sem_trywait(&this->job_queue->num_jobs)) {
				if (errno != EAGAIN) {
					perror("sem_trywait");
					abort();
				}

				job_pending = 0;

				/* No job is pending. Check the kill flag. */
				if (kill_flag) {
					catch_error(pthread_mutex_unlock(&kill_lock));
					return NULL;
				}
			} else {
				/* Just set a local flag here. We can't do anything
				 * slow while we hold the kill_lock mutex
				 */
				job_pending = 1;
			}

			catch_error(pthread_mutex_unlock(&kill_lock));

			/* Now that we released the mutex, handle a job if needed */
			if (job_pending) {
				handle_job(this);
			}

		} while (job_pending);

		/* All immediately pending jobs handled and kill flag checked.
		 * Go into a one second sleep, waking upon a new job
		 */

		/* Create a 1 second timeout */
		struct timespec timeout;
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_sec += 1;

		if (sem_timedwait(&this->job_queue->num_jobs, &timeout)) {
			if (errno != ETIMEDOUT) {
				perror("sem_timedwait");
				abort();
			}
		} else {
			/* Semaphore got set, not timed out */
			handle_job(this);
		}
	}
	return NULL;
}


void * producer_thread(void * param __attribute__ ((unused)))
{
	/* Create Unix domain socket */
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr;

	/* Initialize to 0 */
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	/* Set socket path */
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

	/* Delete socket if it already exists */
	unlink(SOCKET_PATH);

	/* Bind... */
	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
		perror("bind");
		return NULL;
	}

	/* ...and listen. */
	if (listen(sock, 10)) {
		perror("listen");
		return NULL;
	}
	
	char * line = NULL;
	size_t n = 0;
	for (;;) {
		/* Accept connection and handle service
		 * Note that only one client can be serviced at a time. Ideally
		 * this would accept -> fork, but that would be a big damn pain to fit
		 * into the existing threaded architecture.
		 */
		int conn = accept(sock, NULL, NULL);
		if (conn == -1) {
			perror("accept");
			continue;
		}

		FILE * stream = fdopen(conn, "w+");
	
		puts("Client connected.");

		struct printer_group * g;
		struct print_job * job;
		long long job_number = 0;
		for(;;) {
			ssize_t count = getline(&line, &n, stream);
			if (count == -1) {
				if (errno == 0) {
					fputs("Client unexpectedly hung up.\n", stderr);
				} else {
					perror("getline");
				}
				fclose(stream);
				break;
			}

			dprintf("RECV: %s", line);

			if (strncmp(line, "NEW", 3) == 0) {
				job = calloc(1, sizeof(struct print_job));
				job->job_number = job_number++;
			} else if (job && strncmp(line, "FILE", 4) == 0) {
				strtok(line, ": ");
				job->file_name = malloc(n);
				strncpy(job->file_name, strtok(NULL, "\n"), n);
			} else if (job && strncmp(line, "NAME", 4) == 0) {
				strtok(line, ": ");
				job->job_name = malloc(n);
				strncpy(job->job_name, strtok(NULL, "\n"), n);
			} else if (job && strncmp(line, "DESCRIPTION", 11) == 0) {
				strtok(line, ": ");
				job->description = malloc(n);
				strncpy(job->description, strtok(NULL, "\n"), n);
			} else if (job && strncmp(line, "PRINTER", 7) == 0) {
				strtok(line, ": ");
				job->group_name = malloc(n);	
				strncpy(job->group_name, strtok(NULL, "\n"), n);
			} else if (job && strncmp(line, "PRINT", 5) == 0) {
				int group_found = 0;
				if (!job->group_name) {
					eprintf0("Trying to print without setting printer\n");
					continue;
				}
				if (!job->file_name) {
					eprintf0("Trying to print without providing input file\n");
					continue;
				}
				for (g = printer_group_head; g; g=g->next_group) {
					if(strcmp(job->group_name, g->name) == 0) {
						put_job(&g->job_queue, job);
						group_found = 1;
					}
				}
				if (job && !group_found) {
					eprintf("Invalid printer group name given: %s\n", job->group_name);
					continue;
				}
			} else if (strncmp(line, "SHUTDOWN", 8) == 0) {
				exit_flag = 1;
				fclose(stream);
				free(line);
				return NULL;
			} else if (strncmp(line, "EXIT", 4) == 0) {
				puts("Client logged out.");
				fclose(stream);
				break;
			} else if (strncmp(line, "LIST", 4) == 0) {
				send_list(stream);
				fclose(stream);
				break;
			}

		}
	}
}


int main(int argc, char* argv[])
{
	struct printer_group * g;
	struct printer * p;

	// parse the command line arguments
	parse_command_line(argc, argv);

	puts("Starting print server. If it hangs, the drivers probably aren't started.");

	// open the runtime config file
	FILE* config = fopen("config.rc", "r");
	// parse the config file
	parse_rc_file(config);
	// close the config file
	fclose(config);


	//-- Create the consumer threads
	// for each printer group
	for(g = printer_group_head; g; g = g->next_group)
	{
		sem_init(&g->job_queue.num_jobs, 0, 0);
		pthread_mutex_init(&g->job_queue.lock, NULL);
		
		// for each printer in the group
		for(p = g->printer_queue; p; p = p->next)
		{
			// spawn the printer thread
			pthread_create(&p->tid, NULL, printer_thread, (void*)p);
		}
	}
	
	
	//-- Create the prducer thread
	pthread_t producer_tid;
	pthread_create(&producer_tid, NULL, producer_thread, NULL);

	void * ret;
	pthread_join(producer_tid, &ret);

	/** Command all threads to die 
	 * The lock is used to give threads a
	 * chance to do a final check for pending jobs
	 * without risking getting killed.*/
	catch_error(pthread_mutex_lock(&kill_lock));
	kill_flag = 1;
	catch_error(pthread_mutex_unlock(&kill_lock));

	printf("Waiting for all printer threads to finish and terminate\n");

	for (g = printer_group_head; g != NULL; g = g->next_group) {
		for(p = g->printer_queue; p != NULL; p = p->next) {
			void * ret;
			pthread_join(p->tid, &ret);
		}
	}

	unlink(SOCKET_PATH);

	return 0;
}

/**
 * Parse the command line arguments and set the appropriate flags and variables
 * 
 * Recognized arguments:
 *   - `-v`: Turn on Verbose mode
 *   - `-?`: Print help information
 */
static void parse_command_line(int argc, char * argv[])
{
	int c;
	while((c = getopt(argc, argv, "v?l:")) != -1)
	{
		switch(c)
		{
			case 'v': // turn on verbose mode
				verbose_flag = 1;
				break;
			case '?': // print help information
				fprintf(stdout, "Usage: %s [options]\n", argv[0]);
				exit(0);
				break;
			case 'l': // set log file
				logfile = fopen(optarg, "w");
				if (logfile == NULL) {
					perror("fopen");
					abort();
				}
				break;
		}
	}
}

static void parse_rc_file(FILE* fp)
{
	char * line = NULL;
	char * ptr;
	size_t n = 0;
	struct printer_group * group = NULL;
	struct printer_group * g;
	struct printer * printer = NULL;
	struct printer * p;

	// get each line of text from the config file
	while(getline(&line, &n, fp) > 0)
	{
		// if the line is a comment
		if(line[0] == '#')
				continue;

		// If the line is defining a new printer group
		if(strncmp(line, "PRINTER_GROUP", 13) == 0)
		{
			strtok(line, " ");
			ptr = strtok(NULL, "\n");
			group = calloc(1, sizeof(struct printer_group));
			group->name = malloc(strlen(ptr)+1);
			strcpy(group->name, ptr);

			if(printer_group_head)
			{
				for(g = printer_group_head; g->next_group; g=g->next_group);
				g->next_group = group;
			}
			else
			{
				printer_group_head = group;
			}
		}
		// If the line is defining a new printer
		else if(strncmp(line, "PRINTER", 7) == 0)
		{
			strtok(line, " ");
			ptr = strtok(NULL, "\n");
			printer = calloc(1, sizeof(struct printer));
			if (printer_install(&printer->driver, ptr)) {
				fputs("Failed to install printer drivers. Did you start the printer drivers?\n", stderr);
				exit(1);
			}
			printer->job_queue =  &(group->job_queue);
			if(group->printer_queue)
			{
				for(p = group->printer_queue; p->next; p = p->next);
				p->next = printer;
			}
			else
			{
					group->printer_queue = printer;

			}
			num_printers += 1;
		}
	}
	free(line);

	// print out the printer groups
	dprintf0("\n--- Printers ---\n"); 
	for(g = printer_group_head; g; g = g->next_group)
	{
		dprintf("Printer Group %s\n", g->name);
		for(p = g->printer_queue; p; p = p->next)
		{
			dprintf("\tPrinter %s\n", p->driver.name);
		}
	}
	dprintf0("----------------\n\n");

}

/** Simple error catching for teh lulz */
static void catch_error(int err)
{
	if (err) {
		/* Figure that if err is -1, errno is probably already set */
		if (err != -1) {
			errno = err;
		}
		perror("PThread error");
		abort();
	}
}

/** Puts (pushes) a job to the job queue atomically
 * This function increments the queue size semaphore.
 */
static void put_job(struct print_job_list * list, struct print_job * job)
{
	catch_error(pthread_mutex_lock(&list->lock));

	struct print_job * cursor;
	if (list->head == NULL) {
		list->head = job;
	} else {
		for(cursor = list->head; cursor->next_job != NULL; cursor = cursor->next_job) {
			/* Advance pointer to tail */
		}
		cursor->next_job = job;
	}

	catch_error(sem_post(&list->num_jobs));

	catch_error(pthread_mutex_unlock(&(list->lock)));
}

/** Retrieve an item from the queue (atomically)
 * 
 * This function doesn't do any internal checking on
 * the queue semaphore. Make sure that is handled outside, or
 * NULL can be returned if there are no elements.
 */
static struct print_job * get_job(struct print_job_list * list)
{
	catch_error(pthread_mutex_lock(&list->lock));
	struct print_job * ret;

	if (list->head == NULL) {
		/* No-jobs case */
		ret = NULL;
	} else if (list->head->next_job == NULL) {
		/* One job case */
		ret = list->head;
		list->head = NULL;
	} else {
		/* More than one job case */
		struct print_job * cursor;
		for(cursor = list->head; cursor->next_job->next_job != NULL; cursor = cursor->next_job) {
			/* Advance pointer to tail */
		}

		ret = cursor->next_job;
		cursor->next_job = NULL;
	}

	catch_error(pthread_mutex_unlock(&list->lock));
	return ret;
}

/** Helper function since this code happens in 2 places */
static void handle_job(struct printer * this)
{
	struct print_job * job = get_job(this->job_queue);

	time_t start_time = time(NULL);

	if (logfile != NULL) {
		flockfile(logfile);
		fprintf(logfile, "\n-----[%s]-----\n", this->driver.name);
		fprintf(logfile, "Began job %s at %s", job->job_name, asctime(localtime(&start_time)));
		fprintf(logfile, "---------------------------------\n");
		fflush(logfile);
		funlockfile(logfile);
	}

	printer_print(&this->driver, job);

	if (logfile != NULL) {
		time_t finish_time = time(NULL);
		flockfile(logfile);
		fprintf(logfile, "\n-----[%s]-----\n", this->driver.name);
		fprintf(logfile, "Finished job %s at %s(%ds elapsed)\n", job->job_name,
				asctime(localtime(&finish_time)), (int) (finish_time - start_time));
		fprintf(logfile, "--------------------------------\n");
		fflush(logfile);
		funlockfile(logfile);
	}
	free(job->job_name);
	free(job->file_name);
	free(job->description);
	free(job->group_name);
	free(job);
}

static void send_list(FILE * conn)
{
	fprintf(conn, "NUMBER:%d\n", num_printers);

	struct printer_group * gcur = printer_group_head;

	while (gcur != NULL) {
		struct printer * cur = gcur->printer_queue;
		
		while (cur != NULL) {
			fprintf(conn, "%s:%s:%s\n", cur->driver.name, gcur->name, "V1.0-0");
			cur = cur->next;
		}
		gcur = gcur->next_group;
	}
}
