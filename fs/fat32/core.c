#include "fat32.h"
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

static fat32_ctx_t g_ctx;
static media_device_t *g_device = 0;   /* установлен через fat32_set_device() */
static uint8_t g_sector_buf[512];

void fat32_set_device(media_device_t *dev)
{
    g_device = dev;
}

int fat32_mount(void)
{
    fat32_bootsector_t *bs;

    if (!g_device || !g_device->read_sector) {
        return -1; /* устройство не установлено — fat32_set_device() не вызван */
    }

    if (g_device->read_sector(0, g_sector_buf) != 0) {
        return -2;
    }

    bs = (fat32_bootsector_t *)g_sector_buf;

    if (bs->bytes_per_sector != 512) {
        return -3;
    }

    g_ctx.dev = g_device;
    g_ctx.bytes_per_sector = bs->bytes_per_sector;
    g_ctx.sectors_per_cluster = bs->sectors_per_cluster;
    g_ctx.fat_count = bs->fat_count;
    g_ctx.sectors_per_fat = bs->sectors_per_fat32;

    g_ctx.fat_begin_lba = bs->reserved_sectors;
    g_ctx.cluster_begin_lba = g_ctx.fat_begin_lba +
        (g_ctx.fat_count * g_ctx.sectors_per_fat);
    g_ctx.root_dir_first_cluster = bs->root_cluster;

    g_ctx.mounted = 1;
    return 0;
}

fat32_ctx_t *fat32_get_ctx(void)
{
    return g_ctx.mounted ? &g_ctx : 0;
}