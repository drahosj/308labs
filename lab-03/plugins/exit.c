#include <stdio.h>

#include "cash_api.h"

static int builtin_exit(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("exit", &builtin_exit);
}

static int builtin_exit(size_t argc, char ** argv)
{
	if (argc < 2) {
		exit(0);
	} else {
		int exitval;
		sscanf(argv[1], "%d", &exitval);
		exit(exitval);
	}
}
