#ifndef NE2000_H
#define NE2000_H

#include <stdint.h>
#include "netdev.h"

int ne2000_probe(uint16_t io_base);
netdev_t *ne2000_get_device(void);

#endif