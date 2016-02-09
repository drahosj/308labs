#include <stdio.h>
#include <string.h>

#include "cash_api.h"

static size_t num_builtins = 0;

command_type builtins[MAX_BUILTINS];
char commands[MAX_BUILTINS][MAX_COMMAND_LENGTH + 1];

ssize_t register_builtin(char * command, command_type fn)
{
	if (num_builtins < MAX_BUILTINS) {
		builtins[num_builtins] = fn;
		strncpy(commands[num_builtins], command, MAX_COMMAND_LENGTH);
		num_builtins++;
	} else {
		return -1;
	}

	return num_builtins;
}

size_t get_num_builtins()
{
	return num_builtins;
}

command_type get_builtin(char * cmd)
{
	for(size_t i = 0; i < num_builtins; i++) {
		if (strncmp(commands[i], cmd, MAX_COMMAND_LENGTH) == 0) {
			return builtins[i];
		}
	}
	return NULL;
}
