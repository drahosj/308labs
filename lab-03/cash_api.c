#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

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

int load_plugin(char * path)
{
	/* If already open */
	if (dlopen(path, RTLD_NOW | RTLD_NOLOAD) != NULL) {
		return 127;
	}

	void * plugin = dlopen(path, RTLD_NOW);
	if (plugin != NULL) {
		void (*load)(void);

		* (void**) &load = dlsym(plugin, "plugin_load");
		if (load != NULL) {
			(*load)();
		} else {
			fputs("Plugin didn't have a plugin_load function\n", stderr);
			return 1;
		}
	} else {
		fprintf(stderr, "Unable to open  plugin file '%s'\n", path);
		return 2;
	}

	return 0;
}

int load_plugins()
{
      char * line = NULL;
      size_t len = 0;

      FILE * config = fopen("plugins.conf", "r");
      if (config == NULL) {
            perror("fopen");
            return 2;
      }
      while (getline(&line, &len, config) != -1) {
            if ((line[0] == '\n') || (line[0] == '#')) {
                  continue;
            }

            line = strtok(line, "\n");

            load_plugin(line);
      }
      fclose(config);
      return 0;
}
