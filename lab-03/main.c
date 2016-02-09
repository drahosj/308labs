#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>

#include "cash_api.h"

#define MAX_ARGC 32

static int run_shell(FILE * input);
static void usage();

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

	/* Load required shell builtins */
	void * test_plugin = dlopen("plugins/builtins.so", RTLD_NOW);
	if (test_plugin != NULL) {
		void (*load)(void);

	     	* (void**) &load = dlsym(test_plugin, "plugin_load");
		if (load != NULL) {
			(*load)();
		} else {
			fputs("Plugin didn't have a plugin_load function\n", stderr);
		}
	} else {
		fputs("Unable to open builtins plugin file\n", stderr);
	}
	/* END TEMP STUFF */

	int retval;
	retval = run_shell(input);
	fclose(input);
	dlclose(test_plugin);

	return retval;
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

		/* Check for empty line */
		if (line[0] == '\n') {
			continue;
		}

		line = strtok(line, "\n");

		size_t argc;
		char *argv[MAX_ARGC];
		argv[0] = strtok(line, " ");
		for (argc = 1; argc < MAX_ARGC; argc++) {
			argv[argc] = strtok(NULL, " ");
			if (argv[argc] == NULL) {
				break;
			}
		}
		
		command_type builtin;
		builtin = get_builtin(argv[0]);
		if (builtin == NULL) {
			puts(argv[0]);
		} else {
			(*builtin)(argc, argv);
		}
	}
	
	free(line);
	return 0;
}
