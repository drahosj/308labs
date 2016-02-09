#ifndef ___CASH_API_H
#define ___CASH_API_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_BUILTINS 64
#define MAX_COMMAND_LENGTH 32

typedef int (*command_type)(size_t, char **);

ssize_t register_builtin(char * command, command_type);
size_t get_num_builtins();
command_type get_builtin(char * command) ;

#endif
