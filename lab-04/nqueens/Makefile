EXE=nqueens
SRC=nqueens.c
LIB=libresuse.a
LFLAGS=-L./libresuse -pthread -lresuse
CFLAGS=-D_GNU_SOURCE

CC=gcc

OBJ := $(patsubst %.c,%.o,$(SRC))

$(EXE): $(OBJ) 
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


clean:
	rm -f $(OBJ)
	rm -f $(EXE)

