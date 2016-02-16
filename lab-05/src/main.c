/**
 * @file      main.c
 * @author    Jeramie Vens
 * @date      2015-02-11: Created
 * @date      2015-02-15: Last updated
 * @brief     Emulate a print server system
 * @copyright MIT License (c) 2015
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
#include <argp.h>

#include "print_job.h"
#include "printer_driver.h"
#include "debug.h"

int verbose_flag = 0;
int exit_flag = 0;

/// program version string
const char const *argp_program_version = "ISU CprE308 Print Server 0.1";
/// program bug address string
const char const *argp_program_bug_address = "Jeramie Vens: <vens@iastate.edu>";
/// program documentation string
static char doc[] = "Print server -- For my class\vThis should be at the bottom";

// list of options supported
static struct argp_option options[] = 
{
	{"verbose", 'v', 0, 0, "Produce verbose output"},
	{"quite", 'q', 0, 0, "Don't produce any output"},
	{"silent", 's', 0, OPTION_ALIAS, 0},
	{"log-file", 'o', "FILE", 0, "The output log file"},
	{0}
	// The student should add aditional options here
	#warning argp_option not finished
};

static void parse_rc_file(FILE* fp);

static struct printer_group * printer_group_head;

/// arugment structure to store the results of command line parsing
struct arguments
{
	/// are we in verbose mode?
//	int verbose_mode;
	/// name of the log file
	char* log_file_name;
	// The student should add anything else they wish here
	//...
};

/**
 * @brief     Callback to parse a command line argument
 * @param     key
 *                 The short code key of this argument
 * @param     arg
 *                 The argument following the code
 * @param     state
 *                 The state of the arg parser state machine
 * @return    0 if succesfully handeled the key, ARGP_ERR_UNKONWN if the key was uknown
 */
error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	// the student should add the additional required arguments here
	#warning parse_opt not finished
	struct arguments *arguments = state->input;
	switch(key)
	{
		case 'v':
			verbose_flag = 2;
			break;
		case 'q':
			verbose_flag = 0;
			break;
		case 'o':
			arguments->log_file_name = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/// The arg parser object
static struct argp argp = {&options, parse_opt, 0, doc};

struct print_job_list
{
	struct print_job * head;
	sem_t num_jobs;
	pthread_mutex_t lock;
};

struct printer
{
	struct printer * next;
	struct printer_driver driver;
	struct print_job_list * job_queue;
	pthread_t tid;
};


struct printer_group
{
	struct printer_group * next_group;
	char * name;
	struct printer * printer_queue;
	struct print_job_list job_queue;
};


void push_job(struct print_job_list * list, struct print_job * job)
{
	job->next_job = list->head;
	list->head = job;
}

struct print_job * pop_job(struct print_job_list * list)
{
	struct print_job * job, * prev;
	for(job = list->head; job->next_job; prev = job, job = job->next_job);
	prev->next_job = NULL;
	return job;
}

void *printer_thread(void* param)
{
	struct printer * this = param;
	struct print_job * job;
	struct print_job * prev;
	
	printf("I am a thread\n");
	//return NULL;
	while(1)
	{
		// wait for an item to be in the list
		sem_wait(&this->job_queue->num_jobs);
		// lock the list before walking it
		pthread_mutex_lock(&this->job_queue->lock);
		// walk the list to the end
		for(job = this->job_queue->head; job->next_job; prev = job, job = job->next_job);
		if(prev)			
			// fix the tail of the list
			prev->next_job = NULL;
		else
			// There is only one item in the list
			this->job_queue->head = NULL;
			
		// unlock the list
		pthread_mutex_unlock(&this->job_queue->lock);
		
		printf("consumed job %s\n", job->job_name);
		
		// send the job to the printer
		printer_print(&this->driver, job);
		return NULL;
	}

	// The student should fill in all of this function
	#warning printer_spooler not implememnted
	return NULL;
}


void * producer_thread(void * param)
{
	struct printer_group * g;
	struct print_job * job;
	char * line = NULL;
	char * tmp;
	size_t n = 0;
	long long job_number = 0;
	
	while(getline(&line, &n, stdin) > 0)
	{
		if(strncmp(line, "NEW", 3) == 0)
		{
			job = calloc(1, sizeof(struct print_job));
			job->job_number = job_number++;
		}
		else if(job && strncmp(line, "FILE", 4) == 0)
		{
			strtok(line, ": ");
			job->file_name = malloc(n);
			strncpy(job->file_name, strtok(NULL, "\n"), n);
		}
		else if(job && strncmp(line, "NAME", 4) == 0)
		{
			strtok(line, ": ");
			job->job_name = malloc(n);
			strncpy(job->job_name, strtok(NULL, "\n"), n);
		}
		else if(job && strncmp(line, "DESCRIPTION", 11) == 0)
		{
			strtok(line, ": ");
			job->description = malloc(n);
			strncpy(job->description, strtok(NULL, "\n"), n);
		}
		else if(job && strncmp(line, "PRINTER", 7) == 0)
		{
			strtok(line, ": ");
			job->group_name = malloc(n);	
			strncpy(job->group_name, strtok(NULL, "\n"), n);
		}
		else if(job && strncmp(line, "PRINT", 5) == 0)
		{
			if(!job->group_name)
			{
				eprintf("Trying to print without setting printer\n");
				continue;
			}
			if(!job->file_name)
			{
				eprintf("Trying to print without providing input file\n");
				continue;
			}
			for(g = printer_group_head; g; g=g->next_group)
			{
				if(strcmp(job->group_name, g->name) == 0)
				{
					pthread_mutex_lock(&g->job_queue.lock);
					job->next_job = g->job_queue.head;
					g->job_queue.head = job;
					pthread_mutex_unlock(&g->job_queue.lock);
					sem_post(&g->job_queue.num_jobs);
					job = NULL;
					break;
				}
			}
			if(job)
			{
				eprintf("Invalid printer group name given: %s\n", job->group_name);
				continue;
			}
		}
		else if(strncmp(line, "EXIT", 4) == 0)
		{
			exit_flag = 1;
			return NULL;
		}
	}
}


/**
 * @brief     A print server program
 * This program shall take postscript files with some special header information from stdin
 * and print them to a printer device.  For Lab 5 the only printer device you need to support
 * is the pdf_printer provided.  Keep in mind that in future labs you will be expected to
 * support additional printers, so keep your code modular.  All printers will support the
 * API shown in printer.h.  
 *
 * The program should take a number of command line arguments.  At a minimum the following
 * should be supported:
 * - -?, --help: display help information and exit
 * - -V, --version: display version information and exit
 * - -v, --verbose: display debugging information to stderr
 * - -q, --quiet: don't display any messages or outputs unless it is a fatal error
 * - -o, --log-file: use the given log file to print out when jobs start and finish printing
 * - -d, --daemon: future lab will implement this
 * - -c, --config: future lab will implement this
 * - -p, --printer: future lab will implement this
 * - -n1: the number of print queue 1 printers there are (future lab will remove this)
 * - -n2: the number of print queue 2 printers there are (future lab will remove this)
 *
 * The syntax of the postscrip file is as follows.  The file will be suplied through stdin for
 * this lab.  A future lab will change this to a different location, so keep in mind modularity
 * as you go.  Each job will start with header information.  Each line of header information
 * will start with a `#` followed by a keyword and an `=` sign.  You must support at minimum
 * the following commands
 * - #name=: The name of the print job.
 * - #driver=: The driver to print the job to.  For Lab 5 this will be either "pdf1" or "pdf2".
 * - #description=: A discription of the print job which will be included in the log file
 * After all of the header information will be a line starting with a `%`.  Any line following
 * from that line to the line containing `%EOF` should be interpreted as raw postscript data.
 * It should therefore all be copied into the `print_job->file` file.
 *
 * After the `%EOF` has been found a new header may begin for the next file, or if the program
 * is finished `#exit` will be supplied.
 *
 * The flow of the program should be as follows:
 * -# parse command line arguments
 * -# create two print queues using the `queue_create()` function
 * -# install n1 pdf_printer drivers called "pdf1-%d" and n2 pdf_printer drivers called "pdf2-%d"
 * -# create n1+n2 spooler param objects: the first n1 with one of the print queues and all the n1
 *    drivers, and the other n2 with the other print queue and all the n2 drivers
 * -# create n1+n2 spooler threads
 * -# create a new print job using `print_job_create()`
 * -# parse stdin to build the print job
 * -# all postscript data should be appended to the `print_job->file` file
 * -# when the entire job has been read the `print_job->file` file should be closed
 * -# push the print job onto the correct print queue (if the driver was pdf1 or pdf2)
 * -# parse the next print job
 * -# when `#exit` is recieved make sure to release all threads and join them
 * -# free all resources and exit
 *
 * When the program is run with valgrind it should not have ANY memory leaks.  The program
 * should also never cause a segfault for any input or reason.
 */
int main(int argc, char* argv[])
{
	struct printer_group * g;
	struct printer * p;
	// parse arguments.  Look at the man pages and section 25.3 of the GNU libc manual
	// found at https://www.gnu.org/software/libc/manual/pdf/libc.pdf for more information
	// on how to use argp to parse arguments.  An example is shown below to get you started
	struct arguments arguments;
	//arguments.verbose_mode = 1;
	arguments.log_file_name = "";
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

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
	
	
	pthread_t producer_tid;
	pthread_create(&producer_tid, NULL, producer_thread, NULL);

	// for each printer group
	for(g = printer_group_head; g; g = g->next_group)
	{
		// for each printer in the group
		for(p = g->printer_queue; p; p = p->next)
		{
			// spawn the printer thread
			pthread_join(p->tid, NULL);
		}
	}
	return 0;
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

	while(getline(&line, &n, fp) > 0)
	{
		if(line[0] == '#')
				continue;

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
		else if(strncmp(line, "PRINTER", 7) == 0)
		{
			strtok(line, " ");
			ptr = strtok(NULL, "\n");
			printer = calloc(1, sizeof(struct printer));
			printer_install(&printer->driver, ptr);
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
		}
	}

	dprintf("\n--- Printers ---\n"); 
	for(g = printer_group_head; g; g = g->next_group)
	{
		dprintf("Printer Group %s\n", g->name);
		for(p = g->printer_queue; p; p = p->next)
		{
			dprintf("\tPrinter %s\n", p->driver.name);
		}
	}
	dprintf("----------------\n\n");

}

