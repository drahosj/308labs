#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "printserver.h"
#include "print_server_client.h"

static FILE * connect_to_server();

printer_driver_t ** printer_list_drivers(int *number)
{
	FILE * conn = connect_to_server();

	if (fwrite("LIST\n", 1, 5, conn) < 5) {
		perror("write");
	}

	char * line = NULL;
	size_t len = 0;
	ssize_t count = getline(&line, &len, conn);
	if (count == -1) {
		perror("getline");
		fprintf(stderr, "Protocol error\n");
		return NULL;
	}

	*number = 0;
	sscanf(line, "NUMBER:%d", number);
	if (number == 0) {
		fprintf(stderr, "Protocol error\n");
		fclose(conn);
		return NULL;
	}

	printer_driver_t ** drivers = malloc(sizeof(printer_driver_t *) * (*number + 1));
	drivers[*number] = NULL;

	for (int i = 0; i < *number; i++) {
		count = getline(&line, &len, conn);
		if (count == -1) {
			perror("getline");
			fprintf(stderr, "Protocol error\n");
			return NULL;
		}
		char * printer_name = strtok(line, ":");
		if (printer_name == NULL) {
			fprintf(stderr, "Protocol error\n");
			continue;
		}

		char * driver_name = strtok(NULL, ":");
		if (driver_name == NULL) {
			fprintf(stderr, "Protocol error\n");
			continue;
		}

		char * driver_version = strtok(NULL, ":");
		if (driver_version == NULL) {
			fprintf(stderr, "Protocol error\n");
			continue;
		}

		strtok(driver_version, "\n");

		drivers[i] = malloc(sizeof(printer_driver_t));
		drivers[i]->printer_name = printer_name;
		drivers[i]->driver_name = driver_name;
		drivers[i]->driver_version = driver_version;

		/* Next call to getline will create a new buffer */
		line = NULL;
		len = 0;
	}

	fclose(conn);
	return drivers;
}

int printer_print(int * handle __attribute__ ((unused)), char * driver, 
		char * job_name, char * description, char * data)
{
	/* Write data to a temporary file for the printer */
	size_t datasize = strlen(data);	

	char * tmpfile_name = malloc(MAX_TMPFILE_NAME);
	if (tmpfile_name == NULL) {
		perror("malloc");
		abort();
	}

	strcpy(tmpfile_name, TMPFILE_PREFIX);
	strcat(tmpfile_name, job_name);

	FILE * tmpfile = fopen(tmpfile_name, "w");
	if (tmpfile == NULL) {
		perror("fopen");
		free(tmpfile_name);
		abort();
	}

	size_t count = fwrite(data, 1, datasize, tmpfile);
	if (count != datasize) {
		perror("fwrite");
		fclose(tmpfile);
		free(tmpfile_name);
		abort();
	}
	
	if(fclose(tmpfile)) {
		perror("fclose");
		fclose(tmpfile);
		free(tmpfile_name);
		return -1;
	}

	/* Done. Data written to TMPFILE_PREFIX~job_name */

	/* Get connection */
	FILE * conn = connect_to_server();

	/* Send printer commands */
	fprintf(conn, "NEW\n");
	fprintf(conn, "NAME:%s\n",job_name);
	fprintf(conn, "FILE:%s\n",tmpfile_name);
	if (description != NULL) {
		fprintf(conn, "DESCRIPTION:%s\n",description);
	}
	fprintf(conn, "PRINTER:%s\n",driver);
	fprintf(conn, "PRINT\n");
	fprintf(conn, "EXIT\n");
	   
	/* Cleanup */
	free(tmpfile_name);
	fclose(conn);

	/* Success */
	return 0;
}

static FILE * connect_to_server()
{
	/* Open a connection to the socket and send the job */
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);

	/* Initialize address */
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	/* Set address */
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr))) {
		perror("connect");
		abort();
	}

	FILE * conn = fdopen(sock, "w+");
	
	return conn;
}
