#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "cash_api.h"

int builtin_pwd(size_t argc, char ** argv);

void plugin_load()
{
	register_builtin("pwd", &builtin_pwd);
}

int builtin_pwd(__attribute__ ((unused)) size_t argc,
	     __attribute__((unused)) char ** argv)
{
	char * buf = malloc(512);
	if (getcwd(buf, 512) == NULL){
		perror("getcwd");
		free(buf);
		return errno;
	}
	printf("%s\n", buf);
	free(buf);
	return 0;
}
