#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "types.h"

/* Упрощённая структура загрузочного сектора (BPB) FAT32 */
typedef struct {
    uint8_t  jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entry_count;   /* Смещение 17: Должно быть 0 для FAT32 */
    uint16_t total_sectors_16;   /* Смещение 19: Должно быть 0 для FAT32 */
    uint8_t  media_descriptor;   /* Смещение 21 */
    uint16_t sectors_per_fat_16; /* Смещение 22: Должно быть 0 для FAT32 */
    uint16_t sectors_per_track;  /* Смещение 24 */
    uint16_t head_count;         /* Смещение 26 */
    uint32_t hidden_sectors;     /* Смещение 28 */
    uint32_t total_sectors_32;   /* Смещение 32 */

    /* Расширенные поля FAT32 */
    uint32_t sectors_per_fat32;  /* Смещение 36 */
    uint16_t ext_flags;          /* Смещение 40 */
    uint16_t fs_version;         /* Смещение 42 */
    uint32_t root_cluster;       /* Смещение 44 */
} __attribute__((packed)) fat32_bootsector_t;

/* Заглушки функций */
int fat32_mount(void);
int fat32_read_file(const char *path, void *buf, uint32_t max_len);

#endif // FAT32_H
