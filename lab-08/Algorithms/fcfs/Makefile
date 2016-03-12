MOD_NAME = sched-fcfs.mod
SRC = fcfs.c
CFLAGS = -Wall -g -I../../Scheduler
LFLAGS = 

OBJS := $(patsubst %c,%o,$(SRC))

all: $(MOD_NAME)

$(MOD_NAME): $(OBJS)
	gcc -shared -Wl,-soname,$@ $(LFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -o $@ $(CFLAGS) -fPIC -c $<

clean:
	rm -f $(OBJS)
	rm -f $(MOD_NAME)


