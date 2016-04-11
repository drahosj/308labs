# Lab 10: Filesystem 1
## Endianness Test
```
jake at gilliam in ~/308/lab-10 on master*
$ ./endianness_test
little-endian
```
Based on the endianness test, the system I am using
is little endian. This makes sense, since x86_64 is a
little-endian architecture.

### Endian swap
If I ever need to switch endianness, I will use the libc endianness functions
from `man 3 endian`.

## Exercises
### Decoding the boot sector by hand

Boot sector of image.fat12

|                       |Hex            |Decimal        |
|:----------------------|:--------------|:--------------|
|Bytes / sector         | 0x200         | 512           |
|Sectors / cluster      | 0x04          | 4             |
|Root directory entries | 0x200         | 512           |
|Sectors / FAT          | 0x2           | 7             |

## Lab tasks

It was fairly trivial to write a program to decode a FAT boot sector.
The program (fatparse.c) simply opens the image file, does some
sanity checking for size, reads in the boot sector (with more
sanity checking), and then parses out all of the individual values.

Despite both FAT32 and x86 being little-endian, the fatparse program
is written to be portable. It could run without issue on a big-endian
system, thanks to a bit of magic: `le16toh(*((uint16_t *) &boot_sector[OFFSET]))`.
This accomplishes the endian conversion by first casting the array reference
to a uint16_t pointer (boot_sector is a uint8_t array), derefercing it
to pick up a full 16-bit integer in little endian, then passing it to
le16toh to convert it to host format.

This was of course only necessary for the 2-byte values.

### Decoded boot sector
image.fat12
```
jake at gilliam in ~/308/lab-10 on master*
$ ./fatparse image.fat12
| Offset        | Length        |       Contents                |       Value   |
---------------------------------------------------------------------------------
| 0x00          | 3             | Binary offset of boot loader  | EB 3C 90      |
| 0x03          | 8             | Volume label                  | mkfs.fat      |
| 0x0B          | 2             | Bytes per Sector              | 512           |
| 0x0D          | 1             | Sectors per Cluster           | 4             |
| 0x0E          | 2             | Reserved Sectors              | 1             |
| 0x10          | 1             | Number of FATs                | 2             |
| 0x11          | 2             | Number of root dir entries    | 512           |
| 0x13          | 2             | Number of logical sectors     | 2048          |
| 0x15          | 1             | Medium descriptor             | 0x02          |
| 0x16          | 2             | Sectors per FAT               | 2             |
| 0x18          | 2             | Sectors per track             | 32            |
| 0x1A          | 2             | Number of heads               | 64            |
| 0x1C          | 2             | Number of hidden sectors      | 0             |
```

# Conclusion
FAT is indeed a very simple filesystem. The boot sector is especially trivial to
decode, as it only consists of known-length fields at a known offset. Overall, the
hardest part of parsing FAT is formatting the output with printf().
