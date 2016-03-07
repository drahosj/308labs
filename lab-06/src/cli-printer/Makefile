EXE=cli-printer
SRC=main.c
CFLAGS=-D_GNU_SOURCE -g -Wall -Wextra -Wpedantic -Werror -std=gnu11 -I../libprintserver
LFLAGS=-L../libprintserver -lprintserver


OBJ := $(patsubst %.c,%.o,$(SRC))

all: $(EXE)

$(EXE): $(OBJ)
	gcc -o $@ $^ $(LFLAGS)

%.o: %.c *.h
	gcc -c $< $(CFLAGS)

doc:
	doxygen

clean:
	rm -rf *.o
	rm -rf $(EXE)
