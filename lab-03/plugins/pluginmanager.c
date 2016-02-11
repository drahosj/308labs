#include <stdio.h>
#include <stdlib.h>

#include "cash_api.h"

static int builtin_load(size_t argc, char ** argv);
static int builtin_loadall(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("load", &builtin_load);
	register_builtin("loadall", &builtin_loadall);
}

static int builtin_load(size_t argc, char ** argv)
{
	if (argc < 2) {
		return 1;
	}

	return load_plugin(argv[1]);
}

static int builtin_loadall(__attribute__ ((unused)) size_t argc,
		__attribute__ ((unused)) char ** argv)
{
	return load_plugins();
}
