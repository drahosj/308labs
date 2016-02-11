OBJECTS=main.o cash_api.o
CFLAGS=-g -Wall -Werror -Wextra -Wpedantic -std=gnu11

all: cash build-plugins

build-plugins:
	cd plugins && make

cash: $(OBJECTS)
	@echo -e "\n-- Linking cash executable --"
	gcc $(CFLAGS) -o cash -rdynamic -ldl $(OBJECTS)


clean:
	rm -f cash *.o
