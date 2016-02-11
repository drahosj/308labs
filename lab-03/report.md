# Lab 03 Report

This lab was a project to create a Bash-like shell interpreter, known
as Cyclone Advanced SHell, or CASH. It involves utilization of
fork(), exec(), and wait() syscalls, as well as a heavy dose of
command-line parsing and tokenization.

# Lab Questions
## What is the type of the cd command?

cd is a shell builtin

## What is the type of the ls command?

In my bash configuration, ls is an alias to `_ls`, which
itself is a function that calls ls with several options.

## What is the type of the python command?

Python is a normal binary, located at /usr/bin/python


## A note on Shellshock

Exported environment variables are passed to child processes. In Bash, functions
are really just a special type of environment variable, and as such
are passed to children. The vulnerable implementation of Bash had a bug
that would cause it to, when parsing and importing functions passed
in from a parent process, execute a malformed function. This was bad,
because anything that could set environment variable followed by an
invocation of bash could execute code. In particular, Git servers and
CGI servers using Bash were vulnerable, as Git over SSH allows environment
variables to pass through, and CGI runners pass cookies and header data in
as environment variables to the scripts.

The proof of concept export is a variable such as '() { :;}; echo doing bad things'.
Nothing should appear after the second semicolon, but it does. Rather than ignore this
malformed data, the bugged Bash implementations would execute (which is basically
the worst thing it could do in that case).

# Implementation of CASH

CASH has been implemented following the basic requirements of the lab tasks, with a number
of extra credit requirements fulfilled as well.
CASH, when invoked, will check for a shebang and exit if one is not found.i
Errors with the script are handled appropriately, and any errors in the
execution of CASH itself are caught, printed, and cause an exit (Some are
non-fatal and survivable). No segfaults occur, and no resources leak while
CASH is running (Some can be lost upon exit, depending on how it terminates).

All required commands are supported

- cd (Default to home, otherwise specified path. / if $HOME is not set.)
- pwd is implemented as a plugin (more on this later)
- export is implemented as a plugin
- echo is implemented as a plugin
- Any executable in $PATH or as an absolute/relative path will be run, following
normal shell semantics.
- Comments on their own lines are ignored. The # need not be the first character.
- Command lines can have a comment appended. Text following the # is a comment and ignored.
- Backgrounding commands is supported as required. The PID, status, and command are printed
upon termination
- Extra Credit: Variables are supported
- Extra Credit: All builtin commands are implemented as dynamically loaded plugin modules. See
readme.md for more info.
- Extra Credit: The shell can run in interactive mode (-i or --interactive)
