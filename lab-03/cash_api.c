#include <stdio.h>

#include "cash_api.h"

ssize_t num_builtins = 0;

ssize_t register_builtin(char * command, int(*fn)(size_t argc, char ** argv))
{
	char * argv_v = NULL;

	printf("Registering and testing new builtin %s\n", command);

	(*fn)(0, &argv_v);

	return ++num_builtins;
}
