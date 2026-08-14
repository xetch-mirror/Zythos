/* Лёгкий однопотоковый TCP: нет ретрансмита, нет congestion control,
   нет реального окна — просто SYN/ACK/данные/FIN для ОДНОГО
   соединения за раз. Хватит для простого запроса-ответа (например,
   будущий pkg fetch), не для настоящего сетевого стека. */

#include "tcp.h"
#include "network.h"
#include "sys_io.h"

#define TCP_FLAG_SYN 0x02
#define TCP_FLAG_ACK 0x10
#define TCP_FLAG_FIN 0x01
#define TCP_FLAG_PSH 0x08

#pragma pack(push, 1)
typedef struct {
    uint8_t  ver_ihl;
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t  ttl;
    uint8_t  proto;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} ipv4_hdr_t;

typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t  offset;
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} tcp_hdr_t;
#pragma pack(pop)

static uint16_t g_next_local_port = 40000;

static uint16_t hton16(uint16_t v) { return (v << 8) | (v >> 8); }
static uint32_t ip_to_u32(ipv4_address_t ip)
{
    return (ip.bytes[0] << 24) | (ip.bytes[1] << 16) | (ip.bytes[2] << 8) | ip.bytes[3];
}

static void build_tcp_segment(uint8_t *buf, tcp_conn_t *c, uint8_t flags,
                               const void *payload, uint16_t payload_len)
{
    ipv4_hdr_t *ip = (ipv4_hdr_t *)buf;
    tcp_hdr_t *tcp = (tcp_hdr_t *)(buf + sizeof(ipv4_hdr_t));
    uint8_t *data = buf + sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t);

    ip->ver_ihl = 0x45;
    ip->tos = 0;
    ip->total_len = hton16(sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t) + payload_len);
    ip->id = 0;
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->proto = 6; /* TCP */
    ip->checksum = 0; /* TODO: real checksum not computed — many NICs/
                          hosts will drop this; needed before real use */
    ip->src_ip = 0; /* TODO: needs network_get_ipv4_address() filled in */
    ip->dst_ip = hton16(ip_to_u32(c->remote_ip)) << 16 | hton16(ip_to_u32(c->remote_ip) >> 16);

    tcp->src_port = hton16(c->local_port);
    tcp->dst_port = hton16(c->remote_port);
    tcp->seq = c->seq;
    tcp->ack = c->ack;
    tcp->offset = (sizeof(tcp_hdr_t) / 4) << 4;
    tcp->flags = flags;
    tcp->window = hton16(4096);
    tcp->checksum = 0; /* TODO: TCP checksum also unimplemented */
    tcp->urgent = 0;

    if (payload && payload_len) {
        for (uint16_t i = 0; i < payload_len; i++) data[i] = ((uint8_t *)payload)[i];
    }
}

int tcp_connect(tcp_conn_t *c, ipv4_address_t ip, uint16_t port)
{
    c->state = TCP_SYN_SENT;
    c->remote_ip = ip;
    c->remote_port = port;
    c->local_port = g_next_local_port++;
    c->seq = 1;
    c->ack = 0;

    uint8_t buf[sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t)];
    build_tcp_segment(buf, c, TCP_FLAG_SYN, 0, 0);

    if (network_send_frame(buf, sizeof(buf)) != 0) {
        c->state = TCP_CLOSED;
        return -1;
    }

    /* TODO: no retransmit or timeout — caller must poll tcp_on_frame()
       via network_receive_frame() until state == TCP_ESTABLISHED */
    return 0;
}

int tcp_send(tcp_conn_t *c, const void *data, uint32_t len)
{
    if (c->state != TCP_ESTABLISHED) return -1;
    if (len > 1024) len = 1024; /* no fragmentation — single-segment only */

    uint8_t buf[sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t) + 1024];
    build_tcp_segment(buf, c, TCP_FLAG_ACK | TCP_FLAG_PSH, data, (uint16_t)len);

    int ret = network_send_frame(buf, sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t) + len);
    if (ret == 0) c->seq += len;
    return ret;
}

int tcp_recv(tcp_conn_t *c, void *buf, uint32_t max_len)
{
    if (c->state != TCP_ESTABLISHED) return -1;

    uint8_t frame[1500];
    int n = network_receive_frame(frame, sizeof(frame));
    if (n <= 0) return 0;

    tcp_on_frame(c, frame, (uint32_t)n);

    ipv4_hdr_t *ip = (ipv4_hdr_t *)frame;
    tcp_hdr_t *tcp = (tcp_hdr_t *)(frame + sizeof(ipv4_hdr_t));
    uint16_t hdr_total = sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t);
    uint16_t payload_len = hton16(ip->total_len) - hdr_total;

    if (payload_len == 0 || payload_len > max_len) return 0;

    uint8_t *data = frame + hdr_total;
    for (uint16_t i = 0; i < payload_len; i++) ((uint8_t *)buf)[i] = data[i];

    c->ack += payload_len;
    return (int)payload_len;
}

void tcp_on_frame(tcp_conn_t *c, const uint8_t *frame, uint32_t len)
{
    if (len < sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t)) return;

    tcp_hdr_t *tcp = (tcp_hdr_t *)(frame + sizeof(ipv4_hdr_t));

    if (c->state == TCP_SYN_SENT && (tcp->flags & (TCP_FLAG_SYN | TCP_FLAG_ACK))) {
        c->ack = tcp->seq + 1;
        c->seq += 1;
        c->state = TCP_ESTABLISHED;

        uint8_t ackbuf[sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t)];
        build_tcp_segment(ackbuf, c, TCP_FLAG_ACK, 0, 0);
        network_send_frame(ackbuf, sizeof(ackbuf));
    } else if (tcp->flags & TCP_FLAG_FIN) {
        c->state = TCP_CLOSING;
    }
}

void tcp_close(tcp_conn_t *c)
{
    if (c->state != TCP_ESTABLISHED) { c->state = TCP_CLOSED; return; }

    uint8_t buf[sizeof(ipv4_hdr_t) + sizeof(tcp_hdr_t)];
    build_tcp_segment(buf, c, TCP_FLAG_FIN | TCP_FLAG_ACK, 0, 0);
    network_send_frame(buf, sizeof(buf));
    c->state = TCP_CLOSED;
}