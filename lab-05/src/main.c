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

// lab1: print server program as it is
// lab2: make as a daemon, install different printer backends
// lab3: write a kernel module that gets installed as a backend

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>
#include "queue.h"
#include "print_job.h"
#include "printer.h"
#include <argp.h>

/// program version string
const char *argp_program_version = "ISU CprE308 Print Server 0.1";
/// program bug address string
const char *argp_program_bug_address = "Jeramie Vens: <vens@iastate.edu>";
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

/// arugment structure to store the results of command line parsing
struct arguments
{
	/// are we in verbose mode?
	int verbose_mode;
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
			arguments->verbose_mode = 2;
			break;
		case 'q':
			arguments->verbose_mode = 0;
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

/// Parameters to pass to the print spooler thread
struct PRINTER_SPOOLER_PARAM
{
	/// The print queue that this spooler can pull off of
	queue_t print_queue_list;
	/// The printer that this spooler can print to
	printer_t printer_driver;
};

/**
 * @brief     This thread represents a print spooler which directly talks to a single printer
 * @param     param
 *                 The PRINT_SPOOL_PARAM this printer should pull its jobs from, casted to a void*
 * @return    NULL
 *
 * This function should loop forever.  The basic process is as follows:
 * 1) pop a print job from this threads queue
 * 2) if it is NULL return
 * 3) print to the log file that printing of the job is starting
 * 4) call the print method on the printer object of the param and check the return
 * 5) handel errors correctly
 * 6) destroy the print job and get the next
 */
void *printer_spooler(void* param)
{
	// The student should fill in all of this function
	#warning printer_spooler not implememnted
	return NULL;
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
	// parse arguments.  Look at the man pages and section 25.3 of the GNU libc manual
	// found at https://www.gnu.org/software/libc/manual/pdf/libc.pdf for more information
	// on how to use argp to parse arguments.  An example is shown below to get you started
	struct arguments arguments;
	arguments.verbose_mode = 1;
	arguments.log_file_name = "";
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	// The student should fill in all of this section
	#warning main not implemented
	return 0;
}



