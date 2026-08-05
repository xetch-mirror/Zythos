#ifndef MEDIA_H
#define MEDIA_H

#include "types.h"

/* Общий интерфейс для устройств хранения (диск, USB, и т.д.) */
typedef struct {
    const char *name;
    int (*read_sector)(uint32_t lba, void *buf);
    int (*write_sector)(uint32_t lba, const void *buf);
} media_device_t;

/* пока не реализовано */
int media_register(media_device_t *dev);

#endif // MEDIA_H