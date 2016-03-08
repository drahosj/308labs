#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "printserver.h"
#include "print_server_client.h"

const struct option longopts[] = {
	{ "driver", 1, NULL, 'd' },
	{ "output", 1, NULL, 'o' },
	{ "description", 1, NULL, 's' },
	{ "list", 0, NULL, 'l' },
	{ "version", 0, NULL, 'v' },
	{ "usage", 0, NULL, 'u' },
	{ "help", 0, NULL, 'h' },
	{ NULL, 0, NULL, 0 }
};

static ssize_t get_file_size(FILE * f);

static int print_list();
static int print_usage();
static int print_help();
static int print_version();

int main(int argc, char ** argv)
{
	/* Values retrieved from arguments */
	char * driver = NULL;
	char * output = NULL;
	char * description = NULL;
	char * input = NULL;

	int opt;
	while ((opt = getopt_long(argc, argv, "d:o:s:lvuh", longopts, NULL)) != -1) {
		switch((char) opt) {
			case 'd':
				driver = optarg;
				break;
			case 'o':
				output = optarg;
				break;
			case 's':
				description = optarg;
				break;
			case 'l':
				return print_list();
			case 'v':
				return print_version();
			case 'u':
				return print_usage();
			case 'h':
				return print_help();
			case '?':
				fprintf(stderr, "Unrecognized option\n");
				return print_help();
		}
	}

	/* Input is last argument */
	input = argv[optind];

	/* Handle unset options */
	if (input == NULL) {
		fprintf(stderr, "Must specify input file!\n");
		return print_usage();
	}

	if (output == NULL) {
		output = malloc(strlen(input));
		/* Hands-down the best way to remove .ps from the end of a file. */

		char * filename = strrchr(input, '/');
		if (filename == NULL) {
			filename = input;
		} else {
			filename += 1;
		}
		strncpy(output, filename, strlen(filename) - 3);
	}

	if (driver == NULL) {
		print_list();
		printf("Enter driver name: ");
		
		size_t n = 0;
		getline(&driver, &n, stdin);
		strtok(driver, "\n");
	}

	/* Validate driver */
	int num_drivers = 0;
	int found = 0;
	printer_driver_t ** list = printer_list_drivers(&num_drivers);
	for (int i = 0; i < num_drivers; i++) {
		if (!strcmp(driver, list[i]->driver_name)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		fprintf(stderr, "Invalid driver selected. Valid drivers:\n");
		print_list();
		return -1;
	}

	/* Open input file */
	FILE * f = fopen(input, "r");

	ssize_t data_size = get_file_size(f);

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
	
	int result = printer_print(NULL, driver, output, description, data);

	free(data);

	return result;
}

static ssize_t get_file_size(FILE * f)
{
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
	
	return (ssize_t) data_size;
}

static int print_list()
{
	int number = 0;
	printer_driver_t ** list = printer_list_drivers(&number);

	if ((list == NULL) || (number == 0)) {
		return -1;
	}

	printf(" Driver | Name | Driver Version \n");
	for(int i = 0; i < number; i++) {
		printf("%s | %s | %s\n", list[i]->driver_name, 
				list[i]->printer_name, list[i]->driver_version);
		free(list[i]->printer_name);
		free(list[i]);
	}
	return 0;
}

static int print_usage()
{
	fprintf(stderr, "Usage: cli-printer [options] <input-file.ps>\n");
	fprintf(stderr, "       cli-printer -l, --list\n");
	fprintf(stderr, "       cli-printer -h, --help\n");
	fprintf(stderr, "       cli-printer -u, --usage\n");
	return -1;
}

static int print_help()
{
	print_usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: -d, --driver <driver>: Specify driver (from cli-printer --list)\n");
	fprintf(stderr, "         -o, --output <name>: Output file name\n");
	fprintf(stderr, "         -s, --description <description>: Job description\n");
	return -1;
}

static int print_version()
{
	fprintf(stderr, "Version lol\n");
	return -1;
}
