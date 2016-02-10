#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/wait.h>

#include "cash_api.h"

#define MAX_ARGC 32

static int run_shell(FILE * input);
static void usage();
static int load_plugins();

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

	if (load_plugins()) {
		return 2;
	}

	int retval;
	retval = run_shell(input);
	fclose(input);

	return retval;
}

static int load_plugins()
{
	char * line = NULL;
	size_t len = 0;

	FILE * config = fopen("plugins.conf", "r");
	if (config == NULL) {
		perror("fopen");
		return 2;
	}
      while (getline(&line, &len, config) != -1) {
            if ((line[0] == '\n') || (line[0] == '#')) {
                  continue;
            }

		line = strtok(line, "\n");

		void * plugin = dlopen(line, RTLD_NOW);
		if (plugin != NULL) {
			void (*load)(void);

			* (void**) &load = dlsym(plugin, "plugin_load");
			if (load != NULL) {
				(*load)();
			} else {
				fputs("Plugin didn't have a plugin_load function\n", stderr);
			}
		} else {
			fputs("Unable to open  plugin file\n", stderr);
		}

	}
	fclose(config);
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
	
	while(1) {
		if (input == stdin) {
			printf("$ ");
		}

		line_size = getline(&line, &len, input);
		if (line_size < 0) {
			perror("getline");
			free(line);
			return 127;
		} else if (line_size == 0) {
			free(line);
			return 0;
		}

		/* Advance to first non-printing character */
		char * start;
		for(start = line; *start <= ' '; start++) {
			if (*start == '\n') {
				break;
			}
		}

		/* Check for empty line */
		if (start[0] == '\n') {
			continue;
		}

		start = strtok(start, "\n");

		size_t argc;
		char *argv[MAX_ARGC];
		argv[0] = strtok(start, " ");
		for (argc = 1; argc < MAX_ARGC; argc++) {
			argv[argc] = strtok(NULL, " ");
			if (argv[argc] == NULL) {
				break;
			} else if (argv[argc][0] == '#') {
				argv[argc] = NULL;
				break;
			}
		}

		int fg;
		if ((argv[argc - 1][0] == '&') && (strlen(argv[argc - 1]) == 1)) {
			argv[argc - 1] = NULL;
			argc -= 1;
			fg = 0;
		} else {
			fg = 1;
		}

		if (argv[0] == NULL) {
			continue;
		}

		command_type builtin;
		builtin = get_builtin(argv[0]);
		if (builtin == NULL) {
			int status;

			pid_t pid = fork();
			if (pid == 0) {
				execvp(argv[0], argv);
			} else if (pid > 0) {
				if (!fg) {
					fprintf(stderr, "[%d]\n", pid);
				}

				pid_t waitresult;
				do {
					waitresult = waitpid(-1, &status, WNOHANG);
					if (waitresult == -1) {
						perror("waitpid");
						return 3;
					} else if (waitresult == pid) {
						break;
					} else if (waitresult > 0) {
						/* Handle normal exit */
						if (WIFEXITED(status)) {
							fprintf(stderr,
				"Child with pid %d exited with status %d\n", 
				waitresult, WEXITSTATUS(status));
						}

						/* Handle signalled termination */
						if (WIFSIGNALED(status)) {
				fprintf(stderr, "Child with pid %d terminated by signal %d\n",
				waitresult, WTERMSIG(status));	
						}
					}
				} while (fg);
			} else {
				perror("fork");
			}

		} else {
			(*builtin)(argc, argv);
			fflush(stdout);
		}
	}
	
	free(line);
	return 0;
}
