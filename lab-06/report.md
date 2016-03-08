# Lab 6: IPC
## Pipes
### pipe_test
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./pipe_test
My child asked "Are you my mummy?"
And then returned 42
~~~
There is first a delay, then all output appears simultaneously.

### What happens when more than one process tries to write to a pipe at the same time?  
- If the write size is smaller than PIPE_BUF bytes, it will occur atomically with respect
to other processes. The messages will not have other bytes interjected within them.
- If the write size is greater than PIPE_BUF bytes, it may occur in multiple steps. These
steps may be interrupted and another process may write bytes in between.

### How does the output of pipe_test.c change if you move the sleep statement from the child to before the
fgets of the parent?
The output/behavior does not appear to change, since the pipe has the effect of synchronizing
between the parent and child at the point of the fgets/fprintf.

### What is the maximum size of a pipe in Linux since 2.6.11?
65536 bytes

## Named Pipe (FIFO)
### What happens when you run the echo command?
The cat command, which was previously hanging, prints "hello fifo", then exits. The echo
command exits immediately.

### What happens if you run the echo first and then the cat?
The echo command blocks until the cat command is run. When the cat command is run, it prints
immediately, then both commands exit.

### Where is the data sent through the FIFO stored?
The data is buffered in memery, internal to the kernel. No data is stored on the underlying mounted
filesystem; the filesystem is just used as a heirarchy to give a name to the FIFO (henced named
pipes).

### Simple fifo program
The simple fifo program is simplefifo.c. Compile it with 'make simplefifo' and run
it as follows:
~~~
simplefifo listen
~~~
to start a fifo listener. Then,
~~~
simplefifo send
~~~
to start a sender. Everything typed in to the sender
will appear on the listener. Ctrl-c or Ctrl-d to exit.
