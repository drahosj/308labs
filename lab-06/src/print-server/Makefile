EXE=printserver
SRC=main.c printer_driver.c
CFLAGS=-D_GNU_SOURCE -Wall -Wextra -Wpedantic -Werror -std=gnu11 -I../libprintserver
LFLAGS=-pthread
DEBUG=-g


OBJ := $(patsubst %.c,%.o,$(SRC))

all: $(EXE)
	@echo "Building printer drivers (virt-printer)"
	cd printer && make

$(EXE): $(OBJ)
	gcc -o $@ $^ $(LFLAGS)

%.o: %.c *.h
	gcc -c $< $(CFLAGS) $(DEBUG)

doc: 
	doxygen

clean:
	rm -rf *.o
	rm -rf $(EXE)
	
.PHONY: doc
