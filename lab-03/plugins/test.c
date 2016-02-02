#include <stdio.h>

#include "cash_api.h"

static int test_plugin_builtin(size_t argc, char ** argv);

void plugin_load()
{
	printf("Plugin loaded\n");
	register_builtin("plugin_builtin", &test_plugin_builtin);
}

static int test_plugin_builtin(size_t argc, char ** argv)
{
	puts("Test plugin builtin called");
	return printf("It has %zd args.\nFirst arg is %s\n", argc, argv[0]);
}
