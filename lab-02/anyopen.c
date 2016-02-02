#include <stdio.h>
#include <unistd.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Maximum sizes and number of configured extensions */
#define MAX_EXTENSION_LENGTH 10
#define MAX_EXTENSIONS_SUPPORTED 50
#define MAX_COMMAND_LENGTH 50
#define MAX_FILES 30
#define CONFIG_FILE_NAME "extensions.conf"

/* Configuration mapping of extensions to commands */
static size_t extensions_defined = 0;
static char extensions[MAX_EXTENSIONS_SUPPORTED][MAX_EXTENSION_LENGTH + 1];
static char commands[MAX_EXTENSIONS_SUPPORTED][MAX_COMMAND_LENGTH + 1];

/* Static functions */
static const char * get_extension_command(char * exten);
static void print_configuration();
static ssize_t define_extension(char * exten, char * command);

int main(int argc, char *argv[])
{
	char * exten;
	char * command;
	char * line = NULL;
	size_t len = 0;
	pid_t pid;
	FILE * config;
	size_t i;
	size_t children_launched = 0;

	/* Check argc, print usage */
	if (argc < 2) {
		fputs("Usage: anyopen <filename>\n", stderr);
		fputs("       anyopen --print-config\n", stderr);
		return 1;
	}


	/* Open and read configuration file. See extensions.conf */
	config = fopen(CONFIG_FILE_NAME, "r");

	if (config == NULL) {
		perror("fopen");
		fputs("Unable to open config file 'extensions.conf'\n", stderr);
		return 1;
	}

	/* Will print a warning and skip the line if there is a syntax error */
	i = 0;
	while (getline(&line, &len, config) != -1) {
		i++;
		if ((line[0] == '\n') || (line[0] == '#')) {
			continue;
		}

		exten = strtok(line, " ");
		if (exten == NULL) {
			fprintf(stderr, "WARNING: Config syntax error on line %d\n", i);
			continue;
		}
		
		command = strtok(NULL, "\n");
		if (command == NULL) {
			fprintf(stderr, "WARNING: Config syntax error on line %d\n", i);
			continue;
		}

		define_extension(exten, command);
	}
	fclose(config);

	/* Print config and return if specified on command line */
	if ((argc >= 2) && (strcmp("--print-config", argv[1]) == 0)) {
		print_configuration();
		return 0;
	}

	for (i = 1; i < argc; i++) {
		const char * command;

		/* Extract extension from argv */
		exten = strrchr(argv[i], '.');

		if (!exten) {
			fputs("Unable to determine file extension. Skipping.\n", stderr);
			continue;
		}
		exten += 1;

		command = get_extension_command(exten);
		/* Fork and exec */
		pid = fork();
		if (pid == 0) {
			if (command == NULL) {
				fputs("Unrecognized extension. Skipping.\n", stderr);
				continue;
			}

			if (execlp(command, command, argv[i], (char *) NULL) < 0) {
				perror("exec");
				continue;
			}
		} else {
			children_launched++;
			fprintf(stderr, "Launched command '%s' for file '%s' with pid %d\n", command, argv[i], pid);
		}
	}
	
	fprintf(stderr, "Waiting for children to return...\n");
	i = 0;
	while (i < children_launched) {
		int status; 
		if ((pid = wait(&status)) > 0) {
			/* Handle normal exit */
			if (WIFEXITED(status)) {
				fprintf(stderr, "Child with pid %d exited with status %d\n", pid, WEXITSTATUS(status));
			}

			/* Handle signalled termination */
			if (WIFSIGNALED(status)) {
				fprintf(stderr, "Child with pid %d terminated by signal %d\n", pid, WTERMSIG(status));
			}	

			i++;
		} else {
			fputs("wait() return -1. Interesting...\n", stderr);
		}
	}
}

/* Search the list of extensions to find the appropriate command */
static const char * get_extension_command(char * exten)
{
	size_t i;
	for(i = 0; i < extensions_defined; i++) {
		if (strcmp(exten, extensions[i]) == 0) {
			break;
		}
	}

	if (i == extensions_defined) {
		return NULL;
	}
	
	return commands[i];
}

/* Add a new command and extension to the configuration array */
static ssize_t define_extension(char * exten, char * command)
{
	size_t old_index = extensions_defined;

	if (extensions_defined == MAX_EXTENSIONS_SUPPORTED) {
		return -1;
	}

	if ((strlen(exten) > MAX_EXTENSION_LENGTH) || (strlen(command) > MAX_COMMAND_LENGTH)) {
		return -1;
	}

	strcpy(extensions[extensions_defined], exten);
	strcpy(commands[extensions_defined], command);
	
	extensions_defined++;

	return old_index;
}

static void print_configuration()
{
	size_t i;
	for(i = 0; i < extensions_defined; i++) {
		printf("[%02d] '%s' -> '%s'\n", i + 1, extensions[i], commands[i]);
	}
}
