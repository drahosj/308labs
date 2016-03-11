

SRC := $(wildcard .c)

OBJ := $(subst %.c,../obj/%.o,%(SRC))

all: $(OBJ)


../obj/%.o: %.c
	gcc -o $@ -c $^

