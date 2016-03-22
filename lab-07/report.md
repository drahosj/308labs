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
kernel output (console), and the module is no longer loaded.

A note about printk output: Printk does not normally go to standard output. Instead,
it goes into the kernel ringbuffer, and is visible with the `dmesg` command. However,
User Mode Linux, when booted into single user mode, seems to redirect kernel
output to the console TTY output as well. Most 'real' systems will make this
output visible on one of the alternative console TTYs, especially during boot. This means
that loading and unloading these modules on a real system (not UML) will be a completely
silent operation, with the printk messages only showing up in dmesg.

## File Example
### `read()` vs `fops.read`
The read syscall greatly simplifies and abstracts the ideas of files to a single integer:
the file descriptior. This file descriptior (fd) is unique to the process, and hides a lot
of the ugly bits associated with the file. These ugly bits appear kernel-side, in the form
of extra parameters that show up in the prototype of device_read. 

The buffer and length parameters are passed, unmodified, from the buf and count
params of the read() syscall.

The parameter `file`, as opposed to an integer fd, contains a bunch of the ugly details
about the file, such as the process handle, fd within the process, and other
bits of data. The buffer parameter is the pointer (within process address space, hence
the usage of put_user rather than simply the dereference operator) given as the `buf`
parameter to the read() syscall. The offset is the current seek position within the file.
'Real' files will modify the offset as they are read or written, and adjust it as appropriate
when seeking. A stream-oriented device driver, though, can simply leave it untouched if it does
not have a logical concept of position within a file.  

This module could actually make use of the offset by using it as a replacement
for the msg_ptr pointer (See notes).

### Expectations
- The printk info will appear upon loading, as with the hello_world module. This is kernel output,
so it will not be redirectable (it is coming from the kernel itself, NOT from the standard
output of the insmod command) and will appear in dmesg.
- The cat from the file will work correctly. This output is redirectable, since it is actually coming
from cat, and from the file in turn. Nothing is added to dmesg during a read or open.
- Attempting to write to the file will cause the error output to appear. This is kernel output, just
like the insertion message, so it will be non-redirectable and in dmesg.
- Something will get angry and perror about the EINVAL returned by write().
- Unloading the module is silent.
- Everything will go horribly wrong when minor number 1 is used.
- Everything will go horribly wrong when the module is removed.

### Command Output
```
root@nude_organist:/host/308/lab-07/hello-file# insmod hello_file.ko
I was assigned major number 254. To talk to
the driver, create a dev file with
'mknod /dev/cpre308 c 254 0'.
Try various minor numbers. Try to cat and echo to
the device file.
root@nude_organist:/host/308/lab-07/hello-file# mknod /dev/cpre308-0 c 254 0
root@nude_organist:/host/308/lab-07/hello-file# cat /dev/cpre308-0
I already told you 0 times Hello World!
root@nude_organist:/host/308/lab-07/hello-file# cat /dev/cpre308-0
I already told you 1 times Hello World!
root@nude_organist:/host/308/lab-07/hello-file# echo "hello" > /dev/cpre308-0
Sorry, this operation isn't supported.
bash: echo: write error: Invalid argument
root@nude_organist:/host/308/lab-07/hello-file# mknod /dev/cpre308-1 c 254 1
root@nude_organist:/host/308/lab-07/hello-file# cat /dev/cpre308-0
I already told you 3 times Hello World!
root@nude_organist:/host/308/lab-07/hello-file# cat /dev/cpre308-1
I already told you 4 times Hello World!
root@nude_organist:/host/308/lab-07/hello-file# rmmod hello_file.ko
root@nude_organist:/host/308/lab-07/hello-file# cat /dev/cpre308-0
cat: /dev/cpre308-0: No such device or address
root@nude_organist:/host/308/lab-07/hello-file#
```

Most of my expectations were correct. However, things didn't go horribly wrong when a different
minor number was used. Things do go horribly wrong when the module is removed, ie. major number
was wrong. This highlights
the fundamental difference between major and minor numbers: The major number is used by the kernel to
route things to the correct module. The minor number is used within the module however it sees fit. This
module simply does not care about it.

Bonus: dmesg output
```
root@nude_organist:/host/308/lab-07/hello-file# dmesg | tail -n 6
[ 4309.180000] I was assigned major number 254. To talk to
[ 4309.180000] the driver, create a dev file with
[ 4309.180000] 'mknod /dev/cpre308 c 254 0'.
[ 4309.180000] Try various minor numbers. Try to cat and echo to
[ 4309.180000] the device file.
[ 4323.750000] Sorry, this operation isn't supported.
root@nude_organist:/host/308/lab-07/hello-file#
```

### Notes on given hello_file
- The "hello world" printout is actually given by reading the file. This can be confirmed
by redirecting the output of `cat`.
- All other messages (init, exit, write error) are kernel output. They are not coming
from the command, rather from the kernel itself. They cannot be redirected, but they do
show up in `dmesg`.
- The try_module_get(THIS_MODULE) and try_module_put(THIS_MODULE) calls are critically important.
They prevent the module from being unloaded while it a process has the device node open. The kernel
was compilef with `rmmod -f` enabled, though, so it can be force-unloaded. This could very well
cause kernel panics.
- The Device_Open count makes it so that only one process can have the file open at a time. While
this is pretty lame from a usability standpoint, it makes it possible to use a single,
static buffer for preparing the message. Otherwise, a bunch of work would be needed
to maintain dynamically allocated buffers on a per-instance basis, allocated
upon `open()` and deleted upon `close()` or termination.
- On that note, both `close()` and process termination are wrapped up by device_release, which
is why it is called release and not simply close.

By using offp instead of the msg_ptr hack, some unexpected
breakages could be avoided. For example, it would probably be possible to get some weirdness
to happen by doing weird tricks with dup() or fork() that would end up breaking the msg_ptr and
causing read() calls to fight over characters within the message. The offp is meant for
this use case.

To get complete multi-open support, offp would have to be combined with per-open-instance memory
allocation and message formatting/preparation.
