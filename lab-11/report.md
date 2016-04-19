# Lab 12: FAT Filesystem
## Introduction

The purpose of this lab was to more fully explore the FAT12 filesystem. This lab
expands beyond the previous lab, by reading the full filesystem, rather
than simply examining the boot sector/BIOS parameter block. The experience from
the previous lab will be necessary, as the BPB parameters are critical to
successfully parse the filesystem.

# Lab Tasks

The included file `listfat.c` can list all of the files and directories
contained within a FAT12 filesystem. It can be compiled by issuing `make`,
and run with `./listfat <image>` or `./listfat -r <image>`.

## Implementation notes

This implementation fully supports directories which span multiple clusters,
as well as long file names. The recursive mode (`-r` flag) will parse
directories recursively, and print the entries with their full absolute path.

This implementation exhaustively checks return values for error status and will
free all resources and terminate gracefully if an error is encountered.

## Observations while implementing

There are several nuances to implementing FAT12. The file allocation table itself
is a bit of a chore to work around. When reading a file, it must be read
one cluster at a time. After that cluster is read, the FAT must be consulted
to see if there is another cluster in the chain. That cluster must then be read,
and the process will repeat until the end of the chain is reached.

FAT12 in particular is annoying because the FAT entries do not align with byte boundaries,
so some bit-banging magic must be done to retrieve the value from the table.

Long File Names are a horrible and ugly hack.

# Program output

```
jake at gilliam in ~/308/lab-11 on master*
$ ./listfat image.fat12
Name: /CPRE308-.12
Attr: V
2016/03/30 18:11:08
Size: 0

Name: /README
Attr: A
2016/03/30 23:20:24
Size: 60

Name: /SUBDIR1
Attr: D
2016/04/02 17:56:10
Size: 0

Name: /BIG
Attr: A
2016/03/30 23:25:28
Size: 11777

Name: /SUBDIR
Attr: D
2016/03/30 23:26:02
Size: 0

jake at gilliam in ~/308/lab-11 on master*
$ ./listfat -r image.fat12
Name: /CPRE308-.12
Attr: V
2016/03/30 18:11:08
Size: 0

Name: /README
Attr: A
2016/03/30 23:20:24
Size: 60

Name: /SUBDIR1
Attr: D
2016/04/02 17:56:10
Size: 0

Name: /SUBDIR1/.
Attr: D
2016/04/02 17:54:48
Size: 0

Name: /SUBDIR1/..
Attr: D
2016/04/02 17:54:48
Size: 0

Name: /SUBDIR1/ANTOHERFILE
Attr: A
2016/04/02 17:56:10
Size: 82

Name: /BIG
Attr: A
2016/03/30 23:25:28
Size: 11777

Name: /SUBDIR
Attr: D
2016/03/30 23:26:02
Size: 0

Name: /SUBDIR/.
Attr: D
2016/03/30 23:25:50
Size: 0

Name: /SUBDIR/..
Attr: D
2016/03/30 23:25:50
Size: 0

Name: /SUBDIR/FILE
Attr: A
2016/03/30 23:26:50
Size: 65
```

# Conclusion
FAT is a very simple filesystem. Despite some of the nuances, gotchas, and hacks (LFN), it is still
trivial to implement. Though inefficient and not robust, the FAT chain approach is very effective
and simple to implement with a basic, recursive program to walk through the chains.
