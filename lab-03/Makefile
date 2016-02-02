OBJECTS=main.o cash_api.o
CFLAGS=-g -Wall -Werror -pedantic -Wextra -std=gnu11

all: cash

cash: $(OBJECTS)
	@echo -e "\n-- Linking cash executable --"
	gcc $(CFLAGS) -o cash -rdynamic -ldl $(OBJECTS)

clean:
	rm -f cash *.o
