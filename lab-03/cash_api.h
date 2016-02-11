#ifndef ___CASH_API_H
#define ___CASH_API_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_BUILTINS 64
#define MAX_COMMAND_LENGTH 32

/* Builtin commands need to be a function pointer of this
 * prototype.
 */
typedef int (*command_type)(size_t, char **);

/* Register a new builtin command. Ties the given
 * command string to the given function pointer
 */
ssize_t register_builtin(char * command, command_type);

/* Return the number of builtins */
size_t get_num_builtins();

/* Retrieve a builtin. Returns NULL if no
 * builtin exists for that command string. Otherwise,
 * the function pointer to that builtin is returned.
 */
command_type get_builtin(char * command) ;

/* Load a plugin from the given (relative) path */
int load_plugin(char * path);

/* Load all plugins specified in plugins.conf */
int load_plugins();

#endif
