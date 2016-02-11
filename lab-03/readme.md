# CASH - Cyclone Advanced SHell

This is an implementation of the Cyclone Advanced SHell, or CASH.
It is a bash-like shell that contains most essential shell functionality.

# Features

The essential shell features are included to run commands. Normal
shell semantics are used to find executables to run as commands, per
the 'p' versions of exec (execlp(), execvp(), execvpe()). Additionally,
CASH supports a number of commands as shell builtins, which are provided
by the plugin interface.

CASH can be run on a script file or in interactive mode. When running on
a script, CASH can be terminated with Ctrl-C (SIGINT). In interactive mode,
CASH cannot be terminated with Ctrl-C. However, any currently running
foreground process will be terminated. This follows normal shell
behavior with respect to SIGINT.

CASH also supports backgrounding commands by appending '&' to the command name.
These commands will run in the background until complete, and are not
interrupted by Ctrl-C when running in interactive mode. The exit status
of background processes is printed when the process is reaped. If it was
terminated by a signal, the terminating signal is printed instead
of an exit status. This is identified by 'Sig: ' in the exit status line.

# Interactive Mode

CASH will enter interactive mode when passed the -i or --interactive flag.
In interactive mode, CASH will display a primitive prompt, and not die
to Ctrl-C, as mentioned above.

# Plugin Interface

CASH supports a plugin face in cash_api.h. Plugins are provided as .so dynamically
loaded modules. Upon initialization, CASH reads the file 'plugins.conf' in the
current directory to determine which plugins to load. All shell builtins
are provided by plugins. If no plugins are loaded, CASH will be missing a large
amount of essential functionality, notably cd and exit. CASH will warn upon
startup if a configured plugin could not be found.

Plugins are loaded at runtime using dlopen() and dlsym(). A plugin .so is expected
to export plugin_load(), which is called when the plugin is first loaded. When
that is called, the plugin can register one or more function pointers as builtin
commands. The functionality for loading plugins and registering builtins is 
provided by cash_api.h and cash_api.c.

## pluginmanager.so

The pluginmanager.so plugin provides two shell builtins: load and loadall. The
load builtin takes the path to a plugin .so and will load it. Loadall will load
any plugins specified in plugins.conf that were not already loaded for some reason,
ie. they could not be found when CASH started.

To test out pluginmanager and the dynamic plugin system, first build cash. Then,
delete one or more plugins in the 'plugins' directory, such as echocd.so or exit.so.
Note that the builtins will be missing (exit and cd will be missing. Echo is
provided by a system binary if the builtin is missing). The 'type' builtin
can also show that the builtins are missing, unless its plugin was deleted as well.

Once CASH is running, sans the deleted builtins, run make again. This will rebuild
the deleted plugins. Then, run loadall. As long as pluginmanager.so was not one
of the deleted plugins, it will go through and reload any missing plugins. CASH
should now have full builtin functionality, including cd and exit.

Alternatively, plugins.conf can be edited to disable some plugins, then they can
be manually loaded with the 'load' builtin, or re-enabled and loaded with loadall.
Plugins can also be written from scratch using Vim, added to the Makefile, compiled,
added to plugins.conf, and then loaded into CASH. This can all be done using an
interactive CASH instance that is already running, without needing to
restart CASH at any point during the process, to demonstrate the full power
of the dynamic plugin module system.

NOTE: if pluginmanager.so is not loaded at startup, there is no way to load additional
plugins during runtime.

# Variables

CASH supports variables. Variables must be set using the export builtin, and expect
BASH-compatible 'export name= value' syntax. 'export name=value' is also acceptable.
It is not acceptable to have a space between the name and the equals sign.
Variables can be referenced as arguments to functions and builtins, using $NAME to
insert the value of the variable into the command line at that point.

# Testing

The provided script testing.sh will show off basic shell features and comments. Some
in-line comments have been added to demonstrate that feature. testing.sh will also
demonstrate variables. (Make sure to build the lab-02 binaries that testing.sh uses).

Another script, advancedtest.sh, deletes several plugins, then launches a second
instance of CASH. This instance of CASH will complain about missing plugins and
builtins, then recompile the missing plugins and load them, demonstrating full
functionality. This test also shows how the background task handler deals
with signal termination.

Launch CASH with --interactive (or just -i) to play around with the interactive mode.
When running interactively, CASH is not terminated by Ctrl-C, but foreground 
processes are. Test this with sleep 10, then Ctrl-C. It behaves as expected.
Launch a background sleep with sleep 10 &, then another foreground sleep 10. Ctrl-c
will cancel the foreground sleep, but the background sleep will eventually finish. 
(You will have to run other commands such as ls to get CASH to reap the
background tasks, see Bugs).

# Bugs

CASH has no support for spaces in arguments. Neither quotes nor backslash-escaping are
supported. This restriction also applies to spaces in variables. Quotes around strings
are not removed. They are treated as any other printing character.

Due to a bug in the export plugin, 'export name value' assigns the value 'value'
environment variable 'name'. This should not be permitted.

Using a # to denote a mid-line comment will not work if the # is not preceeded by a space,
or if it appears in the middle of a word.

Backgrounding with & will not work if the & is not preceeded by a space, or if it appears
in the middle of a word.

Variables cannot be used as commands, they can only be used as arguments to commands.

CASH does not implement sigchld, so it will not print a status as soon as a background process
terminates. Another command must be launched to trigger CASH to reap on the background
processes. CASH will reap any background process that finishes while a task is running in the foreground,
or that is ready for reaping at the moment a new background task is launched. CASH will not reap
children while it is waiting for input from the user.
