---
title: 'Lab Report: Processes and System Calls'
---

# Introduction

This lab demonstrated basic syscall usage with fork(), exec(), and wait().

# Lab Questions

## What are the return values of fork()?

Fork returns twice with different return values. It returns a nonzero, positive
integer to the process which called fork, and zero to the newly created process.
The return value in the calling process is the PID of the newly created process. In
the event that fork() fails, it will return -1, only once, to the calling process
and no child is created. Errno is set in this case

## How can a program determine if it is the parent or child process?

If the return value is 0, it is the child. If the return value is > 0, it
is the parent. If the return value is < 0, it is the parent, no child was
created, and everything is about to hit the fan.

## How can you use waitpid without making the parent wait for the child to end?

waitpid() can be called with the WNOHANG option to return immediately, whether or not
a child process is ready terminate. In this case, it will immediately return
0 if no children have terminated, or a positive pid if a child has terminated.

## What is the return value of waitpid()?

The PID of the triggering child is returned upon success. 0 is returned if WNOHANG
was specified and no child has changed state. -1 is returned on error, and errno is
set.

## What is the return value of execvp()?

exec() and friends will only return if an error occurs. -1 is returned and errno
is set.

## What is the difference between execl and execlp?

The non-p versions require a full path to a file to be provided. The p versions
accept either a full path or a single command name, then do normal $PATH lookup
to find the path to the file to execute.

# Lab Results

## arg-printer
~~~
jake at gilliam in ~/308/lab-02 on master*
$ ./arg-printer
argv[0]="./arg-printer"
jake at gilliam in ~/308/lab-02 on master*
$ vim lab02.md
jake at gilliam in ~/308/lab-02 on master*
$ ./arg-printer
argv[0]="./arg-printer"
jake at gilliam in ~/308/lab-02 on master*
$ ./arg-printer a b c
argv[0]="./arg-printer"
argv[1]="a"
argv[2]="b"
argv[3]="c"
jake at gilliam in ~/308/lab-02 on master*
$ ./arg-printer --version
argv[0]="./arg-printer"
argv[1]="--version"
~~~

As seen, argv[0] is the command entered to run the program. The 
first argument begins at argv[1], and no parsing is done
to understand flags vs. other forms of arguments.

## example
~~~
jake at gilliam in ~/308/lab-02 on master*
$ ./example
argv[0]="./arg-printer"
argv[1]="arguments"
argv[2]="are"
argv[3]="useful"
Child process finished with return code 4
~~~

## anyopen
Anyopen is implemented using a dynamic configuration file and 
can handle multiple arguments provided. The file 'extensions.conf' is
used to configure the extensions and commands. It is commented and can
be easily understood and edited.

An additional mode --print-config was added. When run with this flag,
the configuration is simply read in, printed, and the program terminates.

Anyopen returns 0 upon success, including a successful --print-config. 1 is returned
upon usage error or error opening the config file.

All output is to stderr to facilitate redirection for separating output of
anyopen from output of the children.

### Test Case
~~~
jake at gilliam in ~/308/lab-02 on master*
$ ./anyopen test.foo test.bar test.t test.f test.long notexisting.long
Launched command 'echo' for file 'test.foo' with pid 17470
Launched command 'echo' for file 'test.bar' with pid 17471
Launched command 'true' for file 'test.t' with pid 17472
Launched command 'false' for file 'test.f' with pid 17473
Launched command 'cat' for file 'test.long' with pid 17474
Launched command 'cat' for file 'notexisting.long' with pid 17475
Waiting for children to return...
test.foo
test.bar
Child with pid 17470 exited with status 0
Child with pid 17471 exited with status 0
cat: Child with pid 17472 exited with status 0
notexisting.longChild with pid 17473 exited with status 1
: No such file or directory
I am a cat
Child with pid 17475 exited with status 1
Child with pid 17474 exited with status 0
~~~

This demonstrates the echo and cat commands, as well as both 0 and nonzero (false) return
statuses. Note the race condition where cat and anyopen fight over stdout.

# Conclusion
This was a solid introduction to fork()ing and exec()ing to launch new processes.
The addition of wait() to the lab highlight synchronization, and also makes
it apparent how shells work - The shell launches a new process, which inherits
stdin, stdout, and stderr from the shell. This allows the process to access the
console. The shell can then patiently wait() for the process to exit, then
retake control of the console to handle more commands.
