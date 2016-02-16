EXE=nqueens
SRC=nqueens.c
LIB=libresuse.a
LFLAGS=-L./libresuse -pthread -lresuse
CFLAGS=-Wall -Wextra -Wpedantic -Werror -std=gnu11 -D_GNU_SOURCE

CC=gcc

OBJ := $(patsubst %.c,%.o,$(SRC))

$(EXE): $(OBJ) 
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f $(OBJ)
	rm -f $(EXE)

