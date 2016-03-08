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
The data is buffered in memory internal to the kernel. No data is stored on the underlying mounted
filesystem; the filesystem is just used as a heirarchy to give a name to the FIFO (henced named
pipes).

### Simple fifo program
The simple fifo program is simplefifo.c. Compile it with 'make simplefifo' and run
it as follows:
~~~
simplefifo listen
~~~
to start a fifo listener. Then (order does not actually matter),
~~~
simplefifo send
~~~
to start a sender. Everything typed in to the sender
will appear on the listener. Ctrl-c or Ctrl-d to exit.

## Socket
Sockets are what I decided to use to implement IPC between cli-printer and the printserver. Full details
in the Print Server IPC section.

###  What are the six types of sockets?
- SOCK_STREAM: In-order, reliable, two-way stream, usually TCP.
- SOCK_DGRAM: Datagram - connectionless and unreliable of a fixed max size, usually UDP.
- SOCK_SEQPACKET: Sequenced, reliable, stream of datagrams with additional requirements.
- SOCK_RAW: Raw network protocol access (ie. directly on top of layer 4 IP). 
Used to implement things like ICMP ping in userspace.
- SOCK_RDM: Reliable datagram without guaranteed ordering.
- SOCK_PACKET: Obsolete and should not be used.

### What are the two domains that can be used for local communications?
AF_UNIX and AF_LOCAL are intended for local communication. AF_LOCAL is obsolete.
AF_INET is often used for local communications via IP localhost (127.0.0.1), quite possibly
more often than AF_UNIX - definitely with more publicity. AF_INET6 can also be used for local
communications via IPv6 loopback (::1), which crops up from time to time. Other domains may also feature
loopback functionality, but they see very little use.

## Message Queues
When mq_test1 is started before mq_test2, the following results are observed in respective terminals:
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./mq_test1
Received message "I am Clara"
~~~
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./mq_test2
Received message "I am the Doctor"
Received message "I am the Master"
~~~
Both programs terminate gracefully.

Some weirdness can occur if only one is started - it can send messages to itself. Depending on
the timing between starting both programs, this weirdness can compound and result in each process
only sending to itself.

### What happens when the order is reversed?
The same results are observed when the order is reversed. Sometimes, (rarely), depending on timing, mq_test1
will print nothing and just hang if it is started after mq_test2.

### Adding a second message
A second message was added. Note: to avoid bugs due to differing message length, mq_test1 was modified to
forcibly insert a null byte into the buffer based on the size returned by mq_receive.
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./mq_test1
Received message "I am Amelia"
Received message "I am Clara"
~~~
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./mq_test2
Received message "I am the Doctor"
Received message "I am the Master"
~~~
The second message prints before the first. This is because of the higher priority assigned to
the second message.

## Shared Memory Space


# Lab Tasks
## Print Server IPC
Print server IPC was implemented using a Unix-domain socket located at /tmp/sock_printserver. This location
is configured in the printserver.h header file, shared between the client library and print server
daemon itself. This socket is used to send jobs, as well as retrieve a listing of available
printers and drivers.

Sockets provided an easy, simple, safe, and extendable solution. Multiple clients can connect to
the server at the same time. The nature of binding and listening to a socket prevents any
multiple-writer problems, since each connection gets a unique file descriptor on the server. The
client simply needs to connect to the socket, then issue commands just as in Lab 05.

Another reason for my preference of sockets is the fact that they can be wrapped in to libc 
file streams. Because an active connection (from listen() or accept()) is simply a file descriptor,
it can be attached to a stream with fdopen(3). This allows the use of fprintf() and getline(), which
greatly reduce the manual memory management overhead of solutions such as manually sprintf()-ing
into a buffer before calling a send function. In particular, getline() is probably near the 
easiest, most convenienc, and *safe* way to receive from anything ever, so being able to use
it with sockets is a major plus.

The only other forms of IPC that can be used with fdopen() are pipe-based (anonymous pipes and
named FIFOs). Both of these have drawbacks: with anonymous pipes, there must be a parent-child relationship
between client and server, or the file descriptors must be sent across a Unix-domain socket anyway. With
named pipes, it can be difficult to distinguish between messages coming from multiple simultaneous clients, and
even more difficult to send messages to the appropriate client. With stream sockets (Unix or otherwise),
listen() and accept() make handling multiple clients a breeze.

## Print Server Client Library
The two required functions were implemented, without the implementation of handle in print_server_print.
The provided Makefile creates and links against libprintserver.a, a static library containing the
two print_server functions.

Care was taken while writing this library to avoid leaking any memory or other resources, including during
an error condition. This means that the library functions can safely be wrapped for error handling and retries.
cli-printer does not attempt to resolve errors, and simply exits, but a future client could do so safely without
worrying about resource leaks internal to the library.

## Print Server as Daemon
The print server was daemonized for this lab, using the -d flag. However, the print server
should not be started manually. An init-style script (printserverctl) is provided in the src directory. This script
can be run with either the 'start' or 'stop' command to start and stop the printserver daemon.

## Command Line Printer Program
cli-printer was implemented, with full support for all required and optional arguments. It does not have
an interactive mode. Usage information can be printed with -u, and full help is available with -h.
