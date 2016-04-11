/* FAT12 parser
 *
 * Jake Drahos
 */


#include <stdio.h>
#include <endian.h>
#include <stdint.h>

/* Invocation: fatparse <disk image> */
int main(int argc, char ** argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <disk>\n", argv[0]);
		return 1;
	}

	FILE * image = fopen(argv[1], "r");

	if (image == NULL) {
		perror("fopen");
		return 1;
	}

	/* Seek to end to measure file size */
	if (fseek(image, 0, SEEK_END)) {
		perror("fseek");
		fclose(image);
		return 1;
	}

	/* Check against minimum size for boot sector */
	if (ftell(image) < 0x1E) {
		fprintf(stderr, "Disk %s is too small to contain a boot sector.\n", argv[1]);
		fclose(image);
		return 1;
	}

	/* Seek back to front */
	if (fseek(image, 0, SEEK_SET)) {
		perror("fseek");
		fclose(image);
		return 1;
	}

	uint8_t boot_sector[30];
	size_t result = fread(boot_sector, 1, 30, image);

	/* Make sure entire boot sector was read */
	if (result < 30) {
		fprintf(stderr, "Failed to read boot sector for %s ", argv[1]);
		if (feof(image)) {
			fprintf(stderr, "(Reached EOF)\n");
		} else {
			perror("fread");
		}
		fclose(image);
		return 1;
	}

	/* Begin interpreting and printing */

	/* Column headers */
	printf("| Offset\t| Length\t|\tContents\t\t|\tValue\t|\n");
	printf("---------------------------------------------------------------------------------\n");

	/* Instructions */
	printf("| 0x00\t\t| 3\t\t| Binary offset of boot loader\t| %2X %2X %2X\t|\n", boot_sector[0x0],
			boot_sector[1], boot_sector[2]);

	/* Label */
	printf("| 0x03\t\t| 8\t\t| Volume label\t\t\t| %-8s\t|\n", &boot_sector[0x3]);

	/* A bunch of 1 and 2 byte decimal values */

	printf("| 0x0B\t\t| 2\t\t| Bytes per Sector\t\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0xB])));
	printf("| 0x0D\t\t| 1\t\t| Sectors per Cluster\t\t| %-8d\t|\n", boot_sector[0xD]);
	printf("| 0x0E\t\t| 2\t\t| Reserved Sectors\t\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0xE])));
	printf("| 0x10\t\t| 1\t\t| Number of FATs\t\t| %-8d\t|\n", boot_sector[0x10]);
	printf("| 0x11\t\t| 2\t\t| Number of root dir entries\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x11])));
	printf("| 0x13\t\t| 2\t\t| Number of logical sectors\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x13])));

	/* Hex */
	printf("| 0x15\t\t| 1\t\t| Medium descriptor\t\t| 0x%02X\t\t|\n", boot_sector[0x10]);

	/* Final decimal values (all 16 bit)*/
	printf("| 0x16\t\t| 2\t\t| Sectors per FAT\t\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x16])));
	printf("| 0x18\t\t| 2\t\t| Sectors per track\t\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x18])));
	printf("| 0x1A\t\t| 2\t\t| Number of heads\t\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x1A])));
	printf("| 0x1C\t\t| 2\t\t| Number of hidden sectors\t| %-8d\t|\n", le16toh(*((uint16_t *) &boot_sector[0x1C])));
	return 0;
}


