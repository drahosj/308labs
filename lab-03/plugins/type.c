#include <stdio.h>

#include "cash_api.h"

static int builtin_type(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("type", &builtin_type);
}

static int builtin_type(size_t argc, char ** argv)
{
	if (argc < 2) {
		return 1;
	}
	
	if (get_builtin(argv[1]) == NULL) {
		puts("Not builtin");
	} else {
		puts("Builtin");
	}
	return 0;
}
