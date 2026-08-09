// fat32_private.h — внутренняя структура, не для внешнего использования
#ifndef FAT32_PRIVATE_H
#define FAT32_PRIVATE_H

#include "media.h"
#include "types.h"

typedef struct {
    media_device_t *dev;
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t root_dir_first_cluster;
    uint32_t sectors_per_fat;
    uint8_t  fat_count;
    int mounted;
} fat32_ctx_t;

fat32_ctx_t *fat32_get_ctx(void);

#endif