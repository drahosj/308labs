
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct fat_extBS_32
{
	//extended fat32 stuff
	uint32_t		table_size_32;
	uint16_t		extended_flags;
	uint16_t		fat_version;
	uint32_t		root_cluster;
	uint16_t		fat_info;
	uint16_t		backup_BS_sector;
	uint8_t 		reserved_0[12];
	uint8_t		drive_number;
	uint8_t 		reserved_1;
	uint8_t		boot_signature;
	uint32_t 		volume_id;
	uint8_t		volume_label[11];
	uint8_t		fat_type_label[8];
	uint8_t		bote_code[420];
	uint16_t	signature;
 
}__attribute__((packed)) fat_extBS_32_t;
 
typedef struct fat_extBS_16
{
	//extended fat12 and fat16 stuff
	uint8_t		bios_drive_num;
	uint8_t		reserved1;
	uint8_t		boot_signature;
	uint32_t		volume_id;
	uint8_t		volume_label[11];
	uint8_t		fat_type_label[8];
	uint8_t		boot_code[448];
	uint16_t	signature;
 
}__attribute__((packed)) fat_extBS_16_t;
 
typedef struct fat_BPM
{
	uint8_t 		bootjmp[3];
	uint8_t 		oem_name[8];
	uint16_t 	    bytes_per_sector;
	uint8_t		sectors_per_cluster;
	uint16_t		reserved_sector_count;
	uint8_t		table_count;
	uint16_t		root_entry_count;
	uint16_t		total_sectors_16;
	uint8_t		media_type;
	uint16_t		table_size_16;
	uint16_t		sectors_per_track;
	uint16_t		head_side_count;
	uint32_t 		hidden_sector_count;
	uint32_t 		total_sectors_32;
 
}__attribute__((packed)) fat_BPM_t;

// A function that takes two bytes as little endian and reorders the bytes as one
// big endian short
// LSB is the first byte that is read
// MSB is the second byte read
uint16_t swap_endian(uint8_t LSB, uint8_t MSB){
	uint16_t ret = (uint16_t)MSB;
	ret = ret << 8;
	ret = ret | (uint16_t)LSB;
	return ret;
}

typedef struct fat_BS
{
	fat_BPM_t bpm;
	union
	{
		fat_extBS_16_t fat12_16;
		fat_extBS_32_t fat32;
	}bs;
}__attribute__((packed)) fat_BS_t;

int main(int argc, const char * argv[])
{

	// 1. Open the file
	// 2. read the boot sector and save the values into the struct
	// 3. print out the elements of the struct
	// Good luck!
	uint8_t a = 0x95;
	uint8_t b = 0x22;
	uint16_t c = swap_endian(a, b);
	printf("c = %x\n", c);
	return 0;
}


