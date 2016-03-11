EXE = main

CPPSRC := $(wildcard *.cpp) $(wildcard */*.cpp)
CPPOBJ := $(patsubst %.cpp,%.cpp.o,$(CPPSRC))

CSRC := $(wildcard *.c) $(wildcard */*.c)
COBJ := $(patsubst %.c,%.c.o,$(CSRC))


all: $(EXE)
	
$(EXE): $(CPPOBJ) $(COBJ)
	g++ -rdynamic -ldl -o $@ $^

%.cpp.o: %.cpp
	g++ -std=c++11 -g -o $@ -c $<

%.c.o: %.c
	gcc -o $@ -c $<


clean:
	rm -f $(CPPOBJ)
	rm -f $(COBJ)
	rm -f main

.PHONY: all $(SUBDIRS)





