


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

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
	log_stream = stdout;	
	debug_stream = stderr;	

	while((c = getopt(argc, argv, "f:d:n:v?")) != -1)
	{
		switch(c)
		{
			case 'f': // log stream file
				break;
			case 'd': // debug stream file
				break;
			case 'n': // printer name
				break;
			case 'v': // turn on verbose mode
				verbose_flag = 1;
				break;
			case '?': // print help information
				fprintf(stderr, "%s\n", argv[0]);
				break;
		}
	}
		
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
	
	printf("job recieved\n"); fflush(stdout);	
	
	return 0;
}

void onExit(int p)
{
	fclose(print_stream);
	remove("./drivers/printer1");
}



