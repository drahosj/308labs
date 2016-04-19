/* listfat.c
 *
 * List the files/directories in a FAT12 filesystem.
 */

#define _BSD_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <endian.h>

static int print_dirent(uint8_t * dirent, int recurse);
static int parse_dir(uint8_t * dir, uint16_t num_entries, int recurse);

int main(int argc, char ** argv)
{
	int ret;
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [-r] <disk>\n", argv[0]);
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
		ret = 1;
		goto close_image;
	}

	/* Check against minimum size for boot sector */
	if (ftell(image) < 0x1E) {
		fprintf(stderr, "Disk %s is too small to contain a boot sector.\n", argv[1]);
		ret = 1;
		goto close_image;
	}

	/* Seek back to front */
	if (fseek(image, 0, SEEK_SET)) {
		perror("fseek");
		ret = 1;
		goto close_image;
	}

	uint8_t boot_sector[30];
	size_t result = fread(boot_sector, 1, 30, image);

	/* Make sure entire boot sector
	 * was read */
	if (result < 30) {
		fprintf(stderr, "Failed to read boot sector for %s ", argv[1]);
		if (feof(image)) {
			fprintf(stderr, "(unexpected EOF)\n");
		} else {
			perror("fread");
		}
		ret = 1;
		goto close_image;
	}

	uint16_t bytes_per_sector = le16toh(*((uint16_t *) &boot_sector[0xB]));
	uint16_t sectors_per_cluster = le16toh(*((uint16_t *) &boot_sector[0xD]));
	uint8_t num_fats = boot_sector[0x10];
	uint16_t num_root_dirents = le16toh(*((uint16_t *) &boot_sector[0x11]));
	uint16_t num_logical_sectors = le16toh(*((uint16_t *) &boot_sector[0x13]));
	uint16_t sectors_per_fat = le16toh(*((uint16_t *) &boot_sector[0x16]));

	if (fseek(image, bytes_per_sector * ((num_fats * sectors_per_fat) + 1), SEEK_SET)) {
		perror("fseek");
		ret = 1;
		goto close_image;
	}

	uint8_t * root_dir = malloc(num_root_dirents * 32);

	if (root_dir == NULL) {
		perror("malloc");
		ret = 1;
		goto close_image;
	}

	if (fread(root_dir, 1, num_root_dirents * 32, image) < (num_root_dirents * 32)) {
		if (feof(image)) {
			fprintf(stderr, "Unexpected end of file.\n");
			ret = 1;
			goto free_root_dir;
		}
		perror("fread");
		ret = 1;
		goto free_root_dir;
	}

	parse_dir(root_dir, num_root_dirents, 0);

	/* Success */
	ret = 0;

	/* Release resources opposite the order they were acquired. This
	 * allows for simple "exception handling" on resource aquisition
	 * failure, as resource acquisitions can be sanely unwound.
	 */
free_root_dir:
	free(root_dir);

close_image:
	fclose(image);

	return ret;	
}

static int print_dirent(uint8_t * dirent, int recurse)
{
	/* Check validitiy */
	if ((dirent[0] == 0) || (dirent[0] == 0xE5)) {
		return 0;
	}

	/* Create name and ext, set null bytes */
	char name[9];
	char ext[4];
	name[8] = '\0';
	ext[3] = '\0';

	/* Copy name and ext to buffers */
	memcpy(name, dirent, 8);
	memcpy(ext, &dirent[8], 3);

	printf((ext[0] != ' ') ? "Name: %s.%s\n" : "Name: %s\n", name, ext);

	/* Create formatting buffer for attrs */
	char attr[7];
	memset(attr, '\0', 7);

	/* Set attribute flags */
	strcat(attr, (dirent[0xB] & 0x01) ? "R" : "");
	strcat(attr, (dirent[0xB] & 0x02) ? "H" : "");
	strcat(attr, (dirent[0xB] & 0x04) ? "S" : "");
	strcat(attr, (dirent[0xB] & 0x08) ? "V" : "");
	strcat(attr, (dirent[0xB] & 0x10) ? "D" : "");
	strcat(attr, (dirent[0xB] & 0x20) ? "A" : "");

	printf("Attr: %s\n", attr);

	/* TODO: Time and date */

	/* Size */
	uint32_t size = le32toh(*((uint32_t *) &dirent[0x1C]));
	printf("Size: %d\n", size);

	/* Handle recursion */
	if ((dirent[0xB] & 0x10) && recurse) {
		uint8_t * dir = malloc(size);
		if (dir == NULL) {
			perror("malloc");
			return 1;
		}

		/* TODO: Seek and read */

		parse_dir(dir, size / 32, 1);
		free(dir);
	}
	return 0;
}

static int parse_dir(uint8_t * dir, uint16_t num_entries, int recurse)
{
	for (int i = 0; i < num_entries; i++) {
		print_dirent(&dir[32 * i], recurse);
	}	
	return 0;
}
