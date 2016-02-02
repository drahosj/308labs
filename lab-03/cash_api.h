#ifndef ___CASH_API_H
#define ___CASH_API_H

#include <stdlib.h>
#include <stdint.h>

ssize_t register_builtin(char * command, int (*fn)(size_t argc, char ** argv));

#endif
