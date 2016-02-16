


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <poll.h>
#include <string.h>

#define PRINT_STREAM_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

int verbose_flag = 0;
FILE* log_stream;
FILE* debug_stream;
FILE* print_stream;

void onExit(int p);

const struct sigaction on_exit_act = {
	.sa_handler = onExit,
	//.sa_sigaction = NULL,
	//.sa_mask = 0,
	//.sa_flags = 0,
	//.sa_restorer = NULL,
};

int main(int argc, char* argv[])
{
	int rv;
	int c;	
	int poll_ret;
	int err;
	int pipefd[2];
	pid_t child;
	struct pollfd *p;
	log_stream = stdout;	
	debug_stream = stderr;
	char *printer_name = NULL;
	char *line = NULL;
	char *temp = NULL;
	char *arguments[4];
	FILE *write_end;

	while((c = getopt(argc, argv, "f:d:n:v?")) != -1)
	{
		switch(c)
		{
			case 'f': // log stream file
				break;
			case 'd': // debug stream file
				break;
			case 'n': // printer name
				printer_name = calloc(11 + strlen(optarg), sizeof(char));
				if(printer_name == NULL){
					perror("calloc");
					abort();
				}
				strncat(printer_name, "./drivers/", 10);
				strncat(printer_name, optarg, strlen(optarg));
				// test
				printf("printer_name: %s\n", printer_name);
				break;
			case 'v': // turn on verbose mode
				verbose_flag = 1;
				break;
			case '?': // print help information
				fprintf(stderr, "%s\n", argv[0]);
				break;
		}
	}

	line = calloc(1024, sizeof(char));
	if(line == NULL){
		perror("calloc");
		abort();
	}
	temp = calloc(512, sizeof(char));
	if(temp == NULL){
		perror("calloc");
		abort();
	}
	
	// replace the constant string with `printer_name` later	
	rv = mkfifo("./drivers/printer1", PRINT_STREAM_MODE);
	if(rv)
	{
		perror("mkfifo()");
		abort();
	}

	sigaction(9,&on_exit_act, NULL);

	printf("Switching to background\n");fflush(stdout);
	daemon(1, 1);

	print_stream = fopen("./drivers/printer1", "r");
	if(!print_stream)
	{
		perror("fopen()");
		abort();
	}

	// generate a pollfd struct
	p = calloc(1, sizeof(struct pollfd));
	if(p == NULL){
		perror("calloc");
		abort();
	}
	// set the file descriptor to the value of the print stream
	p->fd = fileno(print_stream);
	// looking for any input events(high priority or otherwise)
	p->events = POLLIN | POLLPRI;
	
	// 1. watch the fifo/print stream
	// 2. when the print stream is not empty, assume the first line is meta data
	// 3. create an unnamed pipe
	// 4. fork
	// 5. change stdin of the child to be the pipe
	// 6. call `ps2pdf - job_name`
	// 7. copy data from the fifo to the pipe until `##END##` is reached
	// 8. wait for the next job
	while(1){
		// poll the fifo for 100 milliseconds
		poll_ret = poll(p, (nfds_t)1, 100);
		// if the poll returns a positive value
		if(poll_ret > 0){
			// means we have a print job
			printf("job recieved\n"); fflush(stdout);
			// read the first line
			fgets(line, 1024, print_stream);
			// parse out the file name from the first line
			temp = strtok(line, ": ");
			if(temp == NULL){
				printf("invalid format\n"); fflush(stdout);
				continue;
			}
			temp = strtok(NULL, ": ");
			temp[strlen(temp) - 1] = '\0';

			// create the argument array
			arguments[0] = "ps2pdf";
			arguments[1] = "-";
			arguments[2] = strdup(temp);
			arguments[3] = NULL;

			// create an unnamed pipe
			err = pipe(pipefd);
			if(err){
				perror("pipe");
				abort();
			}

			// fork a child
			child = fork();

			// if child
			if(child == 0){
				// reopen stdin file handle using the read end of the pipe
				dup2(pipefd[0], fileno(stdin));
				// call `ps2pdf - job_name`
				execvp(arguments[0], arguments);
			// if parent
			}else{
				// read data from print_stream and write it to the write end of the pipe
				// but only until the line "##END##" is reached
				write_end = fdopen(pipefd[1], "w");
				if(write_end == NULL){
					perror("fdopen");
					abort();
				}
				fgets(line, 1024, print_stream);
				while(strncmp(line, "##END##", 7)){
					fprintf(write_end, "%s", line);
					fgets(line, 1024, print_stream);
				}
				// once "##END##" is reached, read one last time, and close the write end of pipe
				fgets(line, 1024, print_stream);
				fclose(write_end);
			}
		}else if(poll_ret == -1){
			perror("poll");
		}
	}
	
	return 0;
}

void onExit(int p)
{
	fclose(print_stream);
	remove("./drivers/printer1");
}



