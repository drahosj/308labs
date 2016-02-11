#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#include "cash_api.h"

#define MAX_ARGC 32
#define MAX_BG_PROCESSES 64
#define MAX_BG_PROCESS_LENGTH 128

static int run_shell(FILE * input);
static void usage();
static void print_exit(pid_t pid, int status);

static char bg_commands[MAX_BG_PROCESSES][MAX_BG_PROCESS_LENGTH + 1];
static pid_t bg_pids[MAX_BG_PROCESSES];

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
	if (argc != 2) {
		usage();
		return 1;
	}

	FILE * input = stdin;
	if (strcmp(argv[1], "-i") && strcmp(argv[1], "--interactive")) {
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

static void usage()
{
	fputs("Usage: cash -i, --interactive\n       cash <script>\n", stderr);
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
			char * arg = strtok(NULL, " ");
			if (arg == NULL) {
				argv[argc] = NULL;
				break;
			} else if (arg[0] == '#') {
				argv[argc] = NULL;
				break;
			}
			if (arg[0] == '$') {
				arg = getenv(arg + 1);
				if (arg == NULL) {
					arg = "";
				}
			}
			
			argv[argc] = arg;
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

				/* Shouldn't be reached */
				perror("exec");
				exit(errno);
			} else if (pid > 0) {
				if (!fg) {
					fprintf(stderr, "[%d]\n", pid);

					int i = 0;
					while(1) {
						if (i >= MAX_BG_PROCESSES) {
							fputs("ERR: Pid table full, command data lost!\n", stderr);
							break;
						}

						if (bg_pids[i] == 0) {
							bg_pids[i] = pid;
							strncpy(bg_commands[i], argv[0], MAX_BG_PROCESS_LENGTH);
							break;
						}
						i++;
					}
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
						print_exit(waitresult, status);
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

static void print_exit(pid_t pid, int status)
{
	int exit_status;
	char * term;
	if (WIFEXITED(status)) {
		exit_status = WEXITSTATUS(status);
		term = "";
	} else if (WIFSIGNALED(status)) {
		exit_status = WTERMSIG(status);
		term = "Sig:";
	} else {
		exit_status = 0;
		term = "";
	}

	char * command = "ERROR: LOST";
	
	for (size_t i = 0; i < MAX_BG_PROCESSES; i++) {
		if (pid == bg_pids[i]) {
			command = bg_commands[i];
			bg_pids[i] = 0;
			break;
		}
	}

	if (WIFEXITED(status)) {
		fprintf(stderr, "[%d] %s%d\t\t%s\n\n", pid, term, exit_status, command);
	}

}
