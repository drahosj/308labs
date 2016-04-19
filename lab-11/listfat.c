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
#include <getopt.h>
#include <unistd.h>

struct fat_params {
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t sectors_per_fat;
	uint8_t num_fats;
	uint16_t num_root_dirents;
	uint16_t num_logical_sectors;

	uint32_t fat_size;
	uint8_t * fat;

	FILE * image;
};

static int print_dirent(uint8_t * dirent, int recurse, struct fat_params * p, char * path, char ** lfn);
static int parse_dir(uint8_t * dir, uint16_t num_entries, int recurse, struct fat_params * p, char * path);

int main(int argc, char ** argv)
{
	int ret;
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [-r] <disk>\n", argv[0]);
		return 1;
	}

	int recurse = 0;

	int opt;
	while((opt = getopt(argc, argv, "r")) != -1) {
		switch (opt) {
		case 'r':
			recurse = 1;
			break;
		case '?':
			fprintf(stderr, "Usage: %s [-r] <disk>\n", argv[0]);
			break;
		}
	}

	FILE * image = fopen(argv[optind], "r");

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

	struct fat_params p;

	p.bytes_per_sector = le16toh(*((uint16_t *) &boot_sector[0xB]));
	p.sectors_per_cluster = boot_sector[0xD];
	p.num_fats = boot_sector[0x10];
	p.num_root_dirents = le16toh(*((uint16_t *) &boot_sector[0x11]));
	p.num_logical_sectors = le16toh(*((uint16_t *) &boot_sector[0x13]));
	p.sectors_per_fat = le16toh(*((uint16_t *) &boot_sector[0x16]));

	p.fat_size = p.bytes_per_sector * p.sectors_per_fat;

	if (fseek(image, p.bytes_per_sector * ((p.num_fats * p.sectors_per_fat) + 1), SEEK_SET)) {
		perror("fseek");
		ret = 1;
		goto close_image;
	}

	uint8_t * root_dir = malloc(p.num_root_dirents * 32);

	if (root_dir == NULL) {
		perror("malloc");
		ret = 2;
		goto close_image;
	}

	if (fread(root_dir, 1, p.num_root_dirents * 32, image) < (p.num_root_dirents * 32)) {
		if (feof(image)) {
			fprintf(stderr, "Unexpected end of file.\n");
			ret = 1;
			goto free_root_dir;
		}
		perror("fread");
		ret = 1;
		goto free_root_dir;
	}

	p.fat = malloc(p.fat_size);
	if (p.fat == NULL) {
		perror("malloc");
		ret = 2;
		goto free_root_dir;
	}

	if (fseek(image, p.bytes_per_sector, SEEK_SET)) {
		perror("fseek");
		ret = 1;
		goto free_fat;
	}

	if (fread(p.fat, 1, p.fat_size, image) < p.fat_size) {
		if (feof(image)) {
			fprintf(stderr, "Unexpected EOF while reading FAT\n");
			ret = 1;
			goto free_fat;
		}
		perror("fread");
		ret = 1;
		goto free_fat;
	}

	p.image = image;

	parse_dir(root_dir, p.num_root_dirents, recurse, &p, "/");

	/* Success */
	ret = 0;

	/* Release resources opposite the order they were acquired. This
	 * allows for simple "exception handling" on resource aquisition
	 * failure, as resource acquisitions can be sanely unwound.
	 */
free_fat:
	free(p.fat);
free_root_dir:
	free(root_dir);
close_image:
	fclose(image);

	return ret;	
}

static int print_dirent(uint8_t * dirent, int recurse, struct fat_params * p, char * path, char ** lfn)
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

	if (dirent[0xb] == 0x0f) {
		char * this_lfn = malloc(13);
		if (this_lfn == NULL) {
			perror("malloc");
			return 2;
		}

		/* Load LFN (We only support ASCII) */
		this_lfn[0] = dirent[1];
		this_lfn[1] = dirent[3];
		this_lfn[2] = dirent[5];
		this_lfn[3] = dirent[7];
		this_lfn[4] = dirent[9];

		this_lfn[5] = dirent[14];
		this_lfn[6] = dirent[16];
		this_lfn[7] = dirent[18];
		this_lfn[8] = dirent[20];
		this_lfn[9] = dirent[22];
		this_lfn[10] = dirent[24];

		this_lfn[11] = dirent[28];
		this_lfn[12] = dirent[30];

		if (*lfn == NULL) {
			*lfn = this_lfn;
		} else {
			char * tmp = realloc(*lfn, strlen(*lfn) + strlen(this_lfn) + 1);
			if (tmp == NULL) {
				perror("malloc");
				free(this_lfn);
				return 2;
			}
			strcat(*lfn, this_lfn);
			free(this_lfn);
		}

		return 0;
	}

	/* Copy name and ext to buffers */
	memcpy(name, dirent, 8);
	memcpy(ext, &dirent[8], 3);


	char * last;
	while (*(last = (strchr(name, '\0')) - 1) == ' ') {
		*last = '\0';
	}
	while (*(last = (strchr(ext, '\0')) - 1) == ' ') {
		*last = '\0';
	}

	if (*lfn == NULL) {
		printf(ext[0] ? "Name: %s%s.%s\n" : "Name: %s%s\n", path, name, ext);
	} else {
		printf(ext[0] ? "Name: %s%s.%s\n" : "Name: %s%s\n", path, *lfn, ext);
	}

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
	
	uint16_t time = le16toh(*((uint16_t *) &dirent[0x16]));
	uint16_t date = le16toh(*((uint16_t *) &dirent[0x18]));

	int h = (time & 0xF800) >> 11;
	int mi = (time & 0x07E0) >> 5;
	int s = (time & 0x001F) * 2;

	int y = ((date & 0xFE00) >> 9) + 1980;
	int mo = (date & 0x01E0) >> 5;
	int d = (date & 0x001F);

	printf("%04d/%02d/%02d %02d:%02d:%02d\n", y, mo, d, h, mi, s);

	/* Size */
	uint32_t size = le32toh(*((uint32_t *) &dirent[0x1C]));
	printf("Size: %d\n\n", size);

	/* Handle recursion */
	if ((dirent[0xB] & 0x10) && (dirent[0xB] != 0x0F) && (name[0] != '.') && recurse) {
		/* Start with the inital cluster */
		uint16_t current_cluster = le16toh(*((uint16_t *) &dirent[0x1A]));

		uint8_t * dir = NULL;
		int num_clusters = 1;
		do {
			/* Allocate space to read this cluster */
			uint8_t * tmp = realloc(dir, num_clusters * p->bytes_per_sector * p->sectors_per_cluster);
			if (tmp == NULL) {
				perror("realloc");
				free(dir);
				return 1;
			} else {
				dir = tmp;
			}

			/* Get number of sectors used by root dir */
			uint32_t root_dir_sectors = (p->num_root_dirents * 32) / p->bytes_per_sector;

			/* Seek to beginning of this cluster */
			if (fseek(p->image, p->bytes_per_sector * ((current_cluster - 2) * p->sectors_per_cluster +
							p->sectors_per_fat * p->num_fats + 1 + root_dir_sectors) , SEEK_SET)) {
				perror("fseek");
				free(dir);
				return 1;
			}

			uint32_t cluster_size = p->sectors_per_cluster * p->bytes_per_sector;
			if (fread(&dir[cluster_size * (num_clusters - 1)], 1, cluster_size, p->image) < cluster_size) {
				if (feof(p->image)) {
					fprintf(stderr, "Unexpected EOF while reading cluster.\n");
					free(dir);
					return 1;
				}
				perror("fread");
				free(dir);
				return 1;
			}

			/* Check for more clusters in the chain */
			/* Get 2 byte entry from fat table */
			uint16_t fat_ent = le16toh(*((uint16_t *) &p->fat[current_cluster + (current_cluster / 2)]));

			/* Retrieve either first 12 or last 12 bits */
			fat_ent = (current_cluster & 0x01) ? fat_ent >> 4 : fat_ent & 0x0FFF;

			if (fat_ent == 0xFF7) {
				fprintf(stderr, "Bad block encountered. Disk corruption likely.\n");
				free(dir);
				return 1;
			} else if (fat_ent >= 0xFF8) {
				break;
			} else {
				current_cluster = fat_ent;
				num_clusters++;
			}
		} while(1);

		char * newpath = malloc(strlen(path) + strlen(name) + 2);
		if (newpath == NULL) {
			perror("malloc");
			free(dir);
			return 2;
		}

		strcpy(newpath, path);
		strcat(newpath, name);
		strcat(newpath, "/");

		uint32_t num_entries = (num_clusters * (p->bytes_per_sector * p->sectors_per_cluster))  / 32;
		/* Break out on critical failure */;
		if (parse_dir(dir, num_entries, 1, p, newpath) == 2) {
			return 2;
		}
		free(newpath);
		free(dir);
	}
	return 0;
}

static int parse_dir(uint8_t * dir, uint16_t num_entries, int recurse, struct fat_params * p, char * path)
{
	char * lfn = NULL;
	for (int i = 0; i < num_entries; i++) {
		/* Break out on critical failure */
		if (print_dirent(&dir[32 * i], recurse, p, path, &lfn) == 2) {
			return 2;
		}
	}	
	return 0;
}
