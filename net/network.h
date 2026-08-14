#ifndef NETWORK_H
#define NETWORK_H

#include "netdev.h"

typedef struct { uint8_t bytes[4]; } ipv4_address_t;
typedef struct { uint8_t bytes[6]; } mac_address_t;

int network_init(void);
int network_is_initialized(void);

int network_get_mac_address(mac_address_t *mac);
int network_get_ipv4_address(ipv4_address_t *ip);
int network_set_ipv4_address(const ipv4_address_t *ip);

int network_send_frame(const void *data, uint32_t len);
int network_receive_frame(void *buf, uint32_t max_len);

int network_get_frames_sent(void);
int network_get_frames_received(void);

#endif