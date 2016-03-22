# The Linux Kernel

## List five architectures that the Linux kernel supports
- DEC Alpha 64
- HP Precision Architure RISC
- Sun Microsystems SPARC
- ARM (32 and 64 bit)
- Motorola 68000 

## List three filesystem types Linux supports
- btrfs
- ext filesystem family (ext2, ext4, ext3 as a subset of ext4 driver)
- Filesystem over 9p (Plan 9 everything-protocol that can also do filesystems)
- Bonus: ZFS (not included in kernel source tree because of licensing issues)

# Kernel Modules
## Hello World
### Command Output
```
root@nude_organist:/host/308/lab-07/hello-world# lsmod
Module                  Size  Used by
root@nude_organist:/host/308/lab-07/hello-world# modinfo hello_world.ko
filename:       /host/308/lab-07/hello-world/hello_world.ko
description:    A simple hello world driver
author:         Jeramie Vens vens@iastate.edu
license:        GPL
depends:
vermagic:       4.4.3drahos mod_unload
root@nude_organist:/host/308/lab-07/hello-world# insmod hello_world.ko
Hello World!
root@nude_organist:/host/308/lab-07/hello-world# lsmod
Module                  Size  Used by
hello_world              867  0
root@nude_organist:/host/308/lab-07/hello-world# rmmod hello_world.ko
Goodbye World!
root@nude_organist:/host/308/lab-07/hello-world# lsmod
Module                  Size  Used by
root@nude_organist:/host/308/lab-07/hello-world#
```

The base kernel loads no modules. This is why the initial `lsmod` returns no modules in use.
The module info, created from the DRIVER_* definitions and the MODULE_* macros, can be
read from the kernel object and displayed with modinfo. The module is then inserted with
`insmod`. This causes the hello_init to run, because it is defined as the initialization
with module_init.

The module
then appears in the module list, and can be removed with `rmmod`. Like `insmod`, `rmmod`
calls the corresponding module lifecycle function hello_exit. This prints to the
kernel output (stdout), and the module is no longer loaded.

A note about printk output: Printk does not normally go to standard output. Instead,
it goes into the kernel ringbuffer, and is visible with the `dmesg` command. However,
User Mode Linux, when booted into single user mode, seems to redirect kernel
output to the console TTY output as well. Most 'real' systems will make this
output visible on one of the alternative console TTYs, especially during boot.

## File Example
### `read()` vs `fops.read`

### Notes on given hello_file
- The "hello world" printout is actually given by reading the file. This can be confirmed
by redirecting the output of `cat`.
- All other messages (init, exit, write error) are kernel output. They are not coming
from the command, rather from the kernel itself. They cannot be redirected, but they do
show up in `dmesg`.
