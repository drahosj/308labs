#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char ** argv)
{
	if (argc != 2) {
		printf("simplefifo listen or simplefifo send\n");
		return -1;
	}

	mkfifo("/tmp/simplefifo", 0666);

	FILE * in;
	FILE * out;
	if (strcmp(argv[1], "listen") == 0) {
		in = fopen("/tmp/simplefifo", "r");
		out = stdout;
	} else if (strcmp(argv[1], "send") == 0) {
		in = stdin;
		out = fopen("/tmp/simplefifo", "w");
	} else {
		printf("simplefifo listen or simplefifo send\n");
		return -1;
	}

	char * buf = NULL;
	size_t n = 0;
	while (getline(&buf, &n, in) > 0) {
		fputs(buf, out);
		fflush(out);
	}

	unlink("/tmp/simplefifo");
	return 0;
}
