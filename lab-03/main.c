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
	/* If it isn't a flag for interactive, assume it is a path to a script */
	if (strcmp(argv[1], "-i") && strcmp(argv[1], "--interactive")) {
		input = fopen(argv[1], "r");
		if (input == NULL) {
			perror("fopen");
			return 2;
		}

		/* Read first line of the script and check for a shebang.
		 * Not exactly a POSIX way of handling it by any means,
		 * but whatever 
		 */
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
	
	/* Load plugins */
	if (load_plugins()) {
		return 127;
	}

	/* Run the script, clean up, and return */
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
	
	/* Each iteration of this loop is a single line of execution
	 * either of the script or of the interactive shell.
	 */
	while(1) {

		/* Print prompt if interactive */
		if (input == stdin) {
			char * buf = malloc(512);
			if (getcwd(buf, 512) == NULL) {
				perror("getcwd");
			}
			printf("%s$ ", buf);
			free(buf);
		}

		/* Read a single line */
		line_size = getline(&line, &len, input);
		if (line_size < 0) {
			perror("getline");
			free(line);
			return 127;
		} else if (line_size == 0) {
			/* End of file */
			free(line);
			return 0;
		}

		/* Advance to first printing character or newline. (This
		 * will probably break on CRLF line-endings) */
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

		/* Strip trailing newline */
		start = strtok(start, "\n");

		/* Check if entire line is a comment */
		size_t argc;
		char *argv[MAX_ARGC];
		argv[0] = strtok(start, " ");
		if (argv[0][0] == '#') {
			continue;
		}

		/* Parse subsequent words into arguments */
		for (argc = 1; argc < MAX_ARGC; argc++) {
			char * arg = strtok(NULL, " ");
			
			/* Check for special cases */
			if (arg == NULL) {
				/* Out of words. No more args */
				argv[argc] = NULL;
				break;
			} else if (arg[0] == '#') {
				/* Rest of the line is a comment */
				argv[argc] = NULL;
				break;
			} else if (arg[0] == '$') {
				/* Expand the variable */
				arg = getenv(arg + 1);
				if (arg == NULL) {
					arg = "";
				}
			}
			
			/* Assign to argv */
			argv[argc] = arg;
		}

		/* Determine foreground or background */
		int fg;
		if ((argv[argc - 1][0] == '&') && (strlen(argv[argc - 1]) == 1)) {
			argv[argc - 1] = NULL;
			argc -= 1;
			fg = 0;
		} else {
			fg = 1;
		}

		/* Last chance to make sure there isn't an empty line or something */
		if (argv[0] == NULL) {
			continue;
		}

		/* See if it's a builtin */
		command_type builtin;
		builtin = get_builtin(argv[0]);
		if (builtin == NULL) {
			/* It isn't a builtin. Time to fork() and exec() */

			pid_t pid = fork();
			if (pid == 0) {
				execvp(argv[0], argv);

				/* Shouldn't be reached */
				fprintf(stderr, "cash (%s): ", argv[0]);
				perror("exec");
				exit(errno);
			} else if (pid > 0) {
				/* Kick it off in the background. Print the
				 * pid of the spawned process, and store
				 * the process command in the pid table.
				 */
				if (!fg) {
					fprintf(stderr, "[%d]\n", pid);

					/* Store it in the first available entry of
					 * the pid table. Running out of pid table
					 * space isn't a fatal error; we will just
					 * lose the name of this particular process.
					 */
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

				/* Check for children that have terminated. Print
				 * their status if they have.
				 *
				 * If we are a foreground process, it will block on
				 * waitpid(). Once the foreground process exits,
				 * we will stop calling waitpid().
				 *
				 * If we are a background process, waitpid will not
				 * block. However, it will keep being called until
				 * all terminated children have been reaped.
				 */
				int status;
				pid_t waitresult;
				do {
					waitresult = waitpid(-1, &status, fg ? WNOHANG : 0);
					if (waitresult == -1) {
						perror("waitpid");
						return 3;
					} else if (waitresult == pid) {
						break;
					} else if (waitresult > 0) {
						print_exit(waitresult, status);
					}
				} while (waitresult);
			} else {
				perror("fork");
			}

		} else {
			(*builtin)(argc, argv);
		}
	}
	
	free(line);
	return 0;
}

/* Print the status and command of a background process
 * that terminated.
 *
 * Note that this will only occur when another process
 * is launched, not necessarily immediately when the
 * background process finishes.
 */
static void print_exit(pid_t pid, int status)
{
	int exit_status;
	char * term;
	
	/* Get exit status or term sig */
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

	/* Get the command from the pid table */
	char * command = "ERROR: LOST";
	for (size_t i = 0; i < MAX_BG_PROCESSES; i++) {
		if (pid == bg_pids[i]) {
			command = bg_commands[i];
			bg_pids[i] = 0;
			break;
		}
	}

	/* Print formatted pid, exit status, and command */
	if (WIFEXITED(status)) {
		fprintf(stderr, "[%d] %s%d\t\t%s\n\n", pid, term, exit_status, command);
	}

}
