#include <stdio.h>
#include <unistd.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXTENSION_LENGTH 10
#define MAX_EXTENSIONS_SUPPORTED 6
#define MAX_COMMAND_LENGTH 25

static size_t extensions_defined = 0;
static char extensions[MAX_EXTENSION_LENGTH + 1][MAX_EXTENSIONS_SUPPORTED];
static char commands[MAX_COMMAND_LENGTH + 1][MAX_EXTENSIONS_SUPPORTED];

static size_t get_extension_index(char * exten);
static const char * get_extension_filename(size_t extension);
static void print_configuration();
static size_t define_extension(char * exten, char * command);

void main(int argc, char *argv[])
{
	/* Initially a hard-coded array */
	strcpy(extensions[0], "doc");
	strcpy(extensions[1], "odt");
	strcpy(extensions[2], "png");
	strcpy(extensions[3], "txt");
	strcpy(extensions[4], "pdf");
	strcpy(extensions[5], "mp3");

	strcpy(commands[0], "libreoffice");
	strcpy(commands[1], "libreoffice");
	strcpy(commands[2], "eog");
	strcpy(commands[3], "gedit");
	strcpy(commands[4], "evince");
	strcpy(commands[5], "vlc");

	extensions_defined = 6;
	print_configuration();
}

static void print_configuration()
{
	size_t i;
	for(i = 0; i < extensions_defined; i++) {
		printf("[%02d] '%s' -> '%s'\n", i + 1, extensions[i], commands[i]);
	}
}
