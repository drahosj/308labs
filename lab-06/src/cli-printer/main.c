#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "printserver.h"
#include "print_server_client.h"

int main(int argc __attribute__ ((unused)), char ** argv)
{
	FILE * f = fopen(argv[1], "r");
	
	if (fseek(f, 0, SEEK_END)) {
		perror("fseek");
		return -1;
	}

	long data_size = ftell(f);
	if (data_size == -1) {
		perror("ftell");
		return -1;
	}
	if (fseek(f, 0, SEEK_SET)) {
		perror("fseek");
		return -1;
	}

	/* Initialize data buffer */
	char * data = malloc(data_size + 1);
	data[data_size] = '\0'; 

	/* Read file into buffer */
	if (fread(data, 1, data_size, f) != (size_t) data_size) {
		perror("fread");
		return -1;
	}

	if(fclose(f)) {
		perror("fclose");
	}
	
	int result = printer_print(NULL, argv[2], argv[3], argv[4], data);

	free(data);

	return result;
}
