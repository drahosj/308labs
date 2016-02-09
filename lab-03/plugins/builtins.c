#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "cash_api.h"

static int builtin_cd(size_t argc, char ** argv);
static int builtin_echo(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("cd", &builtin_cd);
	register_builtin("echo", &builtin_echo);
}
static int builtin_echo(size_t argc, char ** argv)
{
	for (size_t i = 1; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	puts("");

	return 0;
}
static int builtin_cd(size_t argc, char ** argv)
{
	char * dest = NULL;

	if (argc < 2) {
		dest = getenv("HOME");
	} else {
		dest = argv[1];
	}

	/* Default to root if everything else fails */
	if (dest == NULL) {
		dest = "/";
	}

	if(!chdir(dest)) {
		perror("chdir");
		return -1;
	}

	return 0;
}
