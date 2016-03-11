

SRC := $(wildcard .cpp)

OBJ := $(subst %.cpp,../obj/%.o,$(SRC))


$(shell echo $(OBJ))


all: $(OBJ)
	$(shell echo $(OBJ))


../obj/%.o: %.cpp
	gcc -o $@ -c $^

