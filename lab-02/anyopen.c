#include <stdio.h>
#include <unistd.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXTENSION_LENGTH 3
#define MAX_EXTENSIONS_SUPPORTED 10
#define MAX_COMMAND_LENGTH 25

static size_t extensions_defined = 0;
static char extensions[MAX_EXTENSIONS_SUPPORTED][MAX_EXTENSION_LENGTH + 1];
static char commands[MAX_EXTENSIONS_SUPPORTED][MAX_COMMAND_LENGTH + 1];

static const char * get_extension_command(char * exten);
static void print_configuration();
static size_t define_extension(char * exten, char * command);

int main(int argc, char *argv[])
{
	char * exten;
	pid_t pid;

	/* Standard extensions */
	define_extension("doc", "libreoffice");
	define_extension("odt", "libreoffice");
	define_extension("png", "eog");
	define_extension("txt", "vim");
	define_extension("pdf", "evince");
	define_extension("mp3", "vlc");
	define_extension("foo", "echo");

	if ((argc >= 2) && (strcmp("--print-config", argv[1]) == 0)) {
		print_configuration();
		return 0;
	}

	if (argc < 2) {
		fputs("Usage: anyopen <filename>\n", stderr);
		fputs("       anyopen --print-config\n", stderr);
		return 1;
	}

	exten = strrchr(argv[1], '.');

	if (!exten) {
		fputs("Unable to determine file extension.\n", stderr);
		return 2;
	}

	exten += 1;

	pid = fork();
	if (pid == 0) {
		const char * command = get_extension_command(exten);
		if (!command) {
			fputs("Unrecognized extension!\n", stderr);
			return 3;
		}

		if (execlp(command, command, argv[1], (char *) NULL) == -1) {
			perror("exec");
		}
	} else {
		int status; 
		wait(&status);

		if (WIFEXITED(status)) {
			printf("Child exited with status %d\n", WEXITSTATUS(status));
			return 0;
		}

		if (WIFSIGNALED(status)) {
			printf("Child terminated by signal %d\n", WTERMSIG(status));
			return 0;
		}
	}
}

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

static size_t define_extension(char * exten, char * command)
{
	size_t old_index = extensions_defined;

	if (extensions_defined == MAX_EXTENSIONS_SUPPORTED) {
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
