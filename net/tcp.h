#ifndef TCP_H
#define TCP_H

#include <stddef.h>
#include <stdint.h>
#include "network.h"

typedef enum {
    TCP_CLOSED, TCP_SYN_SENT, TCP_ESTABLISHED, TCP_FIN_WAIT, TCP_CLOSING
} tcp_state_t;

typedef struct {
    tcp_state_t state;
    ipv4_address_t remote_ip;
    uint16_t remote_port;
    uint16_t local_port;
    uint32_t seq;
    uint32_t ack;
} tcp_conn_t;

int tcp_connect(tcp_conn_t *c, ipv4_address_t ip, uint16_t port);
int tcp_send(tcp_conn_t *c, const void *data, uint32_t len);
int tcp_recv(tcp_conn_t *c, void *buf, uint32_t max_len);
void tcp_close(tcp_conn_t *c);

/* called by network_receive_frame's caller loop when a frame arrives */
void tcp_on_frame(tcp_conn_t *c, const uint8_t *frame, uint32_t len);

#endif