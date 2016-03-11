

SRC := $(wildcard .cpp)

OBJ := $(subst %.cpp,../obj/%.o,%(SRC))

all: $(OBJ)


../obj/%.o: %.cpp
	gcc -o $@ -c $^

