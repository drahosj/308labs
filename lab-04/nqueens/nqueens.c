/*
    N-Queens threaded solver
    Copyright (C) 2016 Jeramie Vens

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>

// ResUse library found at github.com/JVens/libresuse
#include "libresuse/resuse.h"

// For color text output
#include "bash_color.h"

// Debug printf.  This works the same as printf; however, the printed output
// is displayed on stderr and it will only print if the verbose flag is true.  
#define dprintf(fmt, ...) 														\
	do{																			\
		if(verbose_flag){														\
			flockfile(stderr);													\
			fprintf(stderr, BASH_FORE_BLUE fmt BASH_RESET, 						\
				##__VA_ARGS__);													\
			funlockfile(stderr);												\
		}																		\
	}while(0);


/// The number of queesn to solve for, default = 8
int num_queens = 8;

/// True if using threads, false otherwise
int threaded_flag = 0;

/// True if verbose output should be given, false otherwise
int verbose_flag = 0;

/// True if boards should be displayed on stdout, false otherwise
int display_flag = 0;

/// The format string used for the libresuse output.
const char const * resuse_fmt = "Real Time: %E, User Time: %U, System Time: %S, CPU Usage: %P\n";

/// Next column to assign
int next_column = 0;

/// Mutex protecting next_column
pthread_mutex_t nc_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Test if placing a queen on the @param rows board at a given @param x - 
 * @param y location will be safe or not.
 */
int is_safe(int rows[num_queens], int x, int y)  
{
	int i;

	if (y == 0)
		return 1;
	
	for (i=0; i < y; ++i) 
	{
		if (rows[i] == x || rows[i] == x + y - i || rows[i] == x - y +i)
		return 0;
	}
	
	return 1;
}
 
/**
 * Display the chess board with queens placement on stdout.
 */
void putboard(FILE* fp, int rows[])  
{
	static int s = 0;
	int x, y;

	flockfile(fp);
	fprintf(fp, "\nSolution #%d:\n", ++s);
	for (y=0; y < num_queens; ++y) 
	{
		for (x=0; x < num_queens; ++x)
			fprintf(fp, x == rows[y] ? "|" BASH_FORE_RED " Q " BASH_RESET : "|   ");
		fprintf(fp, "|\n");
	}
	funlockfile(fp);
}

/**
 * Recursive function that searches for solutions to the nqueens problem.
 */
void queens_helper(int rows[], int y, int *solution_count)
{
	int x;

	for (x=0; x < num_queens; ++x) 
	{
		if (is_safe(rows, x, y)) 
		{
			rows[y] = x;
			if (y == num_queens-1)
			{
					(*solution_count)++;
					if (display_flag)
						putboard(stdout, rows);
			}
			else
			{
				queens_helper(rows, y+1, solution_count);
			}
		}
	}
}

/**
 * Data that is passed into each thread
 */
struct thread_param
{
	// The ID of the thread
	pthread_t tid;
	// The number of the thread
	int thread_num;
	// The number of solutions this thread found
	int solution_count;
};

/**
 * The thread created by the student.
 * @param     p
 *                 A struct thread_param object cast to void *
 * @return    A pointer to an int containing the number of solutions found
 *            cast to a void *.
 */
void* thread(void* p)
{
	struct thread_param * param = (struct thread_param *) p;
	struct resuse resuse;
	int rows[num_queens];
	int column;

	// star collecting resource information about this thread
	resuse_start(&resuse, RESUSE_SCOPE_THREAD);

	// print debug information
	dprintf("Thread %d Starting\n", param->thread_num);

	while(1) {
		/* Obtain the mutex for the next column */
		int err = pthread_mutex_lock(&nc_mutex);
		if (err) {
			fprintf(stderr, "ERR: pthread_mutex_lock returned %d\n", err);
			exit(1);
		}

		/* Take the next column */
		column = next_column; 

		/* Increment the next column */
		next_column++;

		/* NOTE: Even though it *looks* atomic,
		 * local = global++ is anything but.
		 * Without the mutex, two threads
		 * could end up grabbing
		 * the same column, then
		 * both incrementing it and
		 * skipping a column, or any
		 * other sort of weirdness.
		 */

		/* Release mutex */
		err = pthread_mutex_unlock(&nc_mutex);
		if (err) {
			fprintf(stderr, "ERR: pthread_mutex_lock returned %d\n", err);
			exit(1);
		}

		if (column >= num_queens) {
			break;
		}

		dprintf("Thread %d kicking off column %d\n", param->thread_num, column);

		rows[0] = param->thread_num;
		queens_helper(rows, 1, &(param->solution_count));

		dprintf("Thread %d finished column %d, looking for new work...\n", param->thread_num, column);

	}
	// print debug information
	dprintf("Thread %d Finished, found %d solutions\n", param->thread_num, 
					param->solution_count);

	// finish collecting resource information about this thread
	resuse_end(&resuse);

	// print the resource information to stdout
	flockfile(stdout);
	fprintf(stdout, "Thread %02d: ", param->thread_num);
	resuse_fprint(stdout, resuse_fmt, &resuse);
	fflush(stdout);
	funlockfile(stdout);

	// return the number of solutions this thread found
	return (void*) &param->solution_count;
}


int main(int argc, char* argv[])
{
	int i;
	int c;
	int solution_count = 0;
	int num_threads;
	struct thread_param * params;
	struct resuse resuse;
	pthread_t * thread_handles;
	
	// Star collecting resource information about the entire proccess
	resuse_start(&resuse, RESUSE_SCOPE_PROC);
	
	// parse the command line arguments
	while((c = getopt(argc, argv, "n:t:vd")) != -1)
	{
		// for each argument
		switch(c)
		{
			// n = The number of queens
			case 'n':
				num_queens = atoi(optarg);
				break;
			// t = The number of threads
			case 't':
				threaded_flag = 1;
				num_threads = atoi(optarg);
				break;
			// v = Turn on verbose output
			case 'v':
				verbose_flag = 1;
				break;
			// d = Display the chess boards
			case 'd':
				display_flag = 1;
				break;
			case '?':
				if (optopt == 'c')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return -1;
			default:
				abort();
		}
	}

	// print out information about the program
	printf("# Queens = %d, # Threads = %d, Threaded = %s, Verbose = %s, Display = %s\n", 
			num_queens, 
			threaded_flag ? num_threads : 1,
			threaded_flag ? "TRUE" : "FALSE",
			verbose_flag ? "TRUE" : "FALSE",
			display_flag ? "TRUE" : "FALSE");

	// If the threaded option is true
	if(threaded_flag) {
		// Dynamically allocate the the thread parameter objects	
		params = calloc(num_queens, sizeof(struct thread_param));
		if (params == NULL) {
			perror("calloc");
			exit(1);
		}
		thread_handles = calloc(num_queens, sizeof(pthread_t));
		if (thread_handles == NULL) {
			perror("calloc");
			exit(1);
		}
	}


	// for each column of the chess board
	if(!threaded_flag) {
		for(i=0;i<num_queens;i++) {
			int rows[num_queens];
			int tmp_solution_count = 0;
			rows[0] = i;
			queens_helper(rows, 1, &tmp_solution_count);
			solution_count += tmp_solution_count;
		}
	} else {
		for(i = 0; i < num_threads; i++) {
			params[i].thread_num = i;
			int err = pthread_create(&(thread_handles[i]), NULL, thread, &(params[i]));
			if (err) {
				fprintf(stderr, "ERR: pthread_create returned %d\n", err);
				exit(1);
			}
		}

		solution_count = 0;
		for (i = 0; i < num_threads; i++) {
			int err = pthread_join(thread_handles[i], NULL);

			if (err) {
				fprintf(stderr, "ERR: pthread_join returned %d\n", err);
			}
			solution_count += params[i].solution_count;
		}
	}


	// Stop collecting resource usage information
	resuse_end(&resuse);
	// Print out the collected information
	resuse_fprint(stdout, resuse_fmt, &resuse);

	// Print out the final number of solutions that were found
	printf("Solution Count = %d\n", solution_count);

	if (threaded_flag) {
		free(params);
		free(thread_handles);
	}
	return 0;
}

