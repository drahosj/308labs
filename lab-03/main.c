#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cash_api.h"

static int run_shell(FILE * input);
static void usage();
static int test_builtin(size_t argc, char ** argv);

/* Exit values:
 * 0 - Success
 * 1 - Usage
 * 2 - Error opening script
 * 3 - Script file doesn't look like a script
 *
 * 127 - Misc. error
 * */

int main(int argc, char ** argv)
{
	FILE * input = stdin;

	if (argc > 2) {
		usage();
		return 1;
	}

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			perror("fopen");
			return 2;
		}
		
		char * line = NULL;
		size_t len = 0;
		if (getline(&line, &len, input) < 0) {
			perror("getline");
			return 127;
		}

		if (strcmp("#! /bin/cash\n", line) != 0) {
			fprintf(stderr, "'%s' doesn't look like a script! " 
					"(does it have a shebang?)\n", argv[1]);
			free(line);
			return 3;
		}
		free(line);
	}

	/* Register a test builtin */
	register_builtin("test", &test_builtin);

	int retval;
	retval = run_shell(input);
	fclose(input);

	return retval;
}

static int test_builtin(size_t argc, char ** argv)
{
	puts("Hello, world!");
	printf("%zd args, of which the first is %s", argc, argv[0]);
	return 0;
}

static void usage()
{
	fputs("Usage: cash\n       cash <script>\n", stderr);
}

static int run_shell(FILE * input)
{
	char * line = NULL;
	size_t len = 0;
	ssize_t line_size;

	while((line_size = getline(&line, &len, input))) {
		if (line_size < 0) {
			perror("getline");
			free(line);
			return 127;
		}
		fputs(line, stdout);
	}
	
	free(line);
	return 0;
}
