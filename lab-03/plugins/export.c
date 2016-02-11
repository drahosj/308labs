#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "cash_api.h"

static int builtin_export(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("export", &builtin_export);
}

static int builtin_export(size_t argc, char ** argv)
{
	char * name;
	char * value;

	if (argc == 1) {
		return 1;
	}

	name = strtok(argv[1], "=");

	if (name == NULL) {
		return 1;
	}

	/* Try to use something after the =, if that fails
	 * use the next argument. If there is no next
	 * argument, give up and return failure.
	 */
	value = strtok(NULL, "=");
	if (value == NULL) {
		value = argv[2];
		if (value == NULL) {
			return 1;
		}
	}

	if (setenv(name, value, 1) == -1) {
		perror("setenv");
		return errno;
	}

	return 0;
}
