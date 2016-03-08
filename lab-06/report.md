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
### What is the output if you run both at the same time calling shm_test1 first?
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test1
a_string = "I am a buffer in the shared memory area"
an_array[] = {42, 1, 4, 9, 16}
a_ptr = 140731380823312 = "I am a string allocated on main's stack!"
~~~
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test2
a_string = "I am a buffer in the shared memory area"
an_array[] = {42, 1, 4, 9, 16}
Segmentation fault
~~~
shm_test2 segfaults.

### What is the output if you run both at the same time calling shm_test2 first?
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test1
a_string = "I am a buffer in the shared memory area"
an_array[] = {0, 1, 4, 9, 16}
a_ptr = 140722746569056 = "I am a string allocated on main's stack!"
~~~
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test2
a_string = "I am a buffer in the shared memory area"
an_array[] = {0, 1, 4, 9, 16}
Segmentation fault
~~~
The array element is 0 because test2 sets it to 42 before test1 sets it to 0.

### Running each one alone.
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test1
a_string = "I am a buffer in the shared memory area"
an_array[] = {0, 1, 4, 9, 16}
a_ptr = 140735920912608 = "I am a string allocated on main's stack!"
~~~
~~~
jake at gilliam in ~/308/lab-06/ipc-types on master*
$ ./shm_test2
a_string = "I am a buffer in the shared memory area"
an_array[] = {42, 1, 4, 9, 16}
Segmentation fault
~~~
For the shm_test2 alon, the rest of the buffer is 1, 4, 9, 16 as
a result of uninitialized data, rather than anything actually setting them
to those values.

### Why is shm_test2 causing a segfault? How could this be fixed?
shm_test2 segfaults when it tries to read the pointer passed through shared memory. This
fails, because the pointer is set by shm_test1 to point somewhere in its address space.
This pointer is meaningless in the context of the address space of shm_test2. A way to try
to 'fix' this would be ask the kernel to map the shared memory to the same base address
in each process, and then only pass pointers that point to shared memory. But that is not
a guaranteed fix, so the best way is to just avoid passing pointers through shared memory.

### What happens if two applications both try to read and set a variable at the same time?
This is a classic read-modify-write concurrency problem. It can either work out okay (first one runs,
then the other runs), or only one modification will stick because the other process grabbed
the variable before the modifiction was written, causing the earlier modification to be lost.

### How can a shared memory space be deleted from the system?
Shared memory space is marked as deletable with shm_unlink(). It will be deleted once no processes
are attached to it. If there is zombie shared memory space as a result of poorly-written applications
or processes with unexpected termination, the icprm tool can be used to call shm_unlink(), which will
free the space if (or as soon as) no more processes are attached to it.

## Unnamed Semaphores
An unnamed semaphore with initial value 5 can be created in shared memory space as follows:
~~~
sem_init(shared_mem->my_sem, 1, 5);
~~~
The only secret sauce here is making sure the second argument (pshared) is non-zero, meaning
the semaphore is shared across processes.

## Named Semaphores
### How long do semaphores last in the kernel?
Semaphores last in the kernel until sem_unlink() is called and no more processes have the
semaphore opened. Similar to shared memory space, the ipcrm tool can be used to sem_unlink() 
named semaphores manually.

### What causes them to be destroyed?
A semaphore will be destroyed once the following conditions are met:
- sem_unlink() has been called on it
- All processes which called sem_open() on it have since called sem_close() or exited.

### What is the basic process for creating and using named semaphores?
Named semaphores are very simple to create. sem_open() must be called, with the name of the
semaphore, O_CREAT, a mode, and an initial value. This creates the semaphore. Then, other
processes/threads can call sem_open() without O_CREAT or any additional options. All threads which
have opened the semaphore may then call sem_post() and sem_wait().

The semaphore should be sem_unlink()ed once it looks like no more processes will be opening it. It will
then be freed once all processes exit or release it by calling sem_close().

## Signals
### What happens when you try to use Ctrl-c to break out of the infinite loop?
When ctrl-c is pressed, it sends the SIGINT signal. This is registered to my_quit_handler, which
does not quit.

### What is the signal number that Ctrl-c sends?
Ctrl-c sends SIGINT (value 2).

### When a process forks, does the child still use the same signal handler?
Across a fork(), the signal dispositions remain the same.

### Hom about during an exec call?
When a process exec()s, handled signals have their handlers reset to the default. However, 
ignored signals remain ignored.

## Dynamic Libraries
When the library was changed, the output of lib_test changed without needing recompilation. If
libhello were created as a statically linked library (.a instead of .so, also different compilation flags),
libhello would need to be re-linked, but not re-compiled.

LD_LIBRARY_PATH is an important environment variable because it (and its friends including LD_PRELOAD)
can drastically change the behavior of dynamically linked executables. This is especially relevant
when security is concerned. LD_LIBRARY_PATH and LD_PRELOAD are ignored for Set-UID executables like
sudo, su, ping, and mount. This prevents malicious library code from being linked into those binaries
and then executing with elevated permissions. 

The configuration file equivalents of LD_LIBRARY_PATH and LD_PRELOAD (/etc/ld.so.conf and 
/etc/ld.so.preload, respectively) still apply even to SUID binaries and can be used
to override normal linking priorities universally, across all binaries.

Another way around LD_LIBRARY_PATH is to use linker flags to set hardcoded library paths within
the output binary itself. This can be done on a per-library basis, which will make the binary use a specific
library and not do any searching within LD_LIBRARY_PATH or otherwise. ELF binaries can also simply contain
additional library search paths, which operate in an almost identical fashion to the LD_LIBRARY_PATH environment
variable.

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
can be run with either the 'start' or 'stop' command to start and stop the printserver daemon. A pidfile
is used to track the status of the printserver daemon.

## Command Line Printer Program
cli-printer was implemented, with full support for all required and optional arguments. It does not have
an interactive mode. Usage information can be printed with -u, and full help is available with -h. Note
that the short option flag -s was used for -description, rather than -d (which is taken by --driver).

# Building, Running, and Testing
## Building
Build all programs as follows:
~~~
cd src
make
~~~
The top-level Makefile will build all three subfolders. It also supports the clean target.

## Running
NOTE: Do not run the printserver binary as a daemon directly. Use printserverctl. See Print Server as Daemon 
for more infurmation.

Start the printserver daemon with the following command (assuming you are in the src directory).
~~~
./printserverctl start
~~~
This will keep track of the printserver daemon pid, as well as automatically manage the printer drivers.

To stop the daemon, just run
~~~
./printserverctl stop
~~~

The client can be run as expected, with all basic flags supported and functional.

## Testing
samplec.ps is provided in the src directory for testing. Print outputs (pdf files) appear in
src/print-server/printer. 

Some example tests:
~~~
jake at gilliam in ~/308/lab-06/src on master*
$ ./printserverctl start
Starting printserver
Starting print server. If it hangs, the drivers probably are not started.
~~~
The print server is now running.

~~~
jake at gilliam in ~/308/lab-06/src/cli-printer on master*
$ ./cli-printer --list
 Driver | Name | Driver Version
--------------------------------
black_white | ./drivers/printer0-r | V1.0-0
black_white | ./drivers/printer1-r | V1.0-0
black_white | ./drivers/printer2-r | V1.0-0
black_white | ./drivers/printer3-r | V1.0-0
color | ./drivers/printer4-r | V1.0-0
color | ./drivers/printer5-r | V1.0-0
~~~

~~~
jake at gilliam in ~/308/lab-06/src/cli-printer on master*
$ ./cli-printer -o example.pdf -d black_white ../samplec.ps
Job submitted
~~~
The file example.pdf is created in ../print-server/printer.

~~~
jake at gilliam in ~/308/lab-06/src/cli-printer on master*
$ ./cli-printer -o example2.pdf ../samplec.ps
 Driver | Name | Driver Version
--------------------------------
black_white | ./drivers/printer0-r | V1.0-0
black_white | ./drivers/printer1-r | V1.0-0
black_white | ./drivers/printer2-r | V1.0-0
black_white | ./drivers/printer3-r | V1.0-0
color | ./drivers/printer4-r | V1.0-0
color | ./drivers/printer5-r | V1.0-0
Enter driver name: color
Job submitted
~~~
The file example2.pdf is created in ../print-server/printer.

~~~
jake at gilliam in ~/308/lab-06/src on master*
$ ./printserverctl stop
Stopping printserver
~~~
The print server is now stopped
