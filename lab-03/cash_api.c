#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "cash_api.h"

static size_t num_builtins = 0;

command_type builtins[MAX_BUILTINS];
char commands[MAX_BUILTINS][MAX_COMMAND_LENGTH + 1];

/* Add a builtin to the table of builtins */
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

/* Search the table of builtins for the given command, then
 * return the function pointer of the builtin
 */
command_type get_builtin(char * cmd)
{
	for(size_t i = 0; i < num_builtins; i++) {
		if (strncmp(commands[i], cmd, MAX_COMMAND_LENGTH) == 0) {
			return builtins[i];
		}
	}
	return NULL;
}

/* Load a given plugin (.so). The plugin_load
 * initialization routine will be called, unless that
 * plugin has already been loaded.
 */
int load_plugin(char * path)
{
	/* If already open */
	if (dlopen(path, RTLD_NOW | RTLD_NOLOAD) != NULL) {
		return 127;
	}

	/* Open the library */
	void * plugin = dlopen(path, RTLD_NOW);
	if (plugin != NULL) {
		void (*load)(void);

		/* Get the plugin_load initialization routine. This
		 * normally causes the plugin to regester one or more
		 * builtins.
		 */
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

/* Read the plugins.conf configuration file and
 * load all of the plugins specified in that file. As
 * with load_plugin, an already loaded plugin will
 * not be reinitialized.
 */
int load_plugins()
{
      char * line = NULL;
      size_t len = 0;

      FILE * config = fopen("plugins.conf", "r");
      if (config == NULL) {
            perror("fopen");
            return 2;
      }

	/* One plugin per line. Blank lines
	 * and comments permitted.
	 */
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
