#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "types.h"

/* Boot Sector структура FAT32 (упрощённая) */
typedef struct {
    uint8_t  jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint32_t sectors_per_fat32;
    uint32_t root_cluster;
} __attribute__((packed)) fat32_bootsector_t;

/* пока не реализовано */
int fat32_mount(void);
int fat32_read_file(const char *path, void *buf, uint32_t max_len);

#endif // FAT32_H