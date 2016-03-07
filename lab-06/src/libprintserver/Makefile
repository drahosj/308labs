CFLAGS=-D_GNU_SOURCE -Wall -Wextra -Wpedantic -Werror -std=gnu11 -I../libprintserver

all: libprintserver.a

libprintserver.a: libprintserver.c
	gcc $(CFLAGS) -c libprintserver.c
	ar rcs libprintserver.a libprintserver.o

clean:
	rm -f libprintserver.a libprintserver.o
