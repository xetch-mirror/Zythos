/* Лёгкий сетевой слой поверх netdev_t. Нет lwIP, нет TCP/UDP, нет
   сокетов — только приём/передача сырых Ethernet-кадров через
   зарегистрированный драйвер NIC. ARP/IP/TCP добавятся позже как
   отдельные слои поверх network_send_frame/network_receive_frame. */

#include "network.h"
#include "netdev.h"
#include "sys_io.h"

static netdev_t *g_dev = 0;
static int g_initialized = 0;

static ipv4_address_t g_ip = {{0, 0, 0, 0}};

static uint32_t g_frames_sent = 0;
static uint32_t g_frames_received = 0;

int network_init(void)
{
    if (g_initialized) return 0;

    g_dev = netdev_get_default();
    if (!g_dev) {
        io_print("[net] no NIC registered\n");
        return -1;
    }

    g_initialized = 1;
    io_print("[net] network layer up on ");
    io_print(g_dev->name);
    io_print("\n");
    return 0;
}

int network_is_initialized(void)
{
    return g_initialized;
}

int network_get_mac_address(mac_address_t *mac)
{
    if (!g_initialized || !mac) return -1;
    for (int i = 0; i < 6; i++) mac->bytes[i] = g_dev->mac[i];
    return 0;
}

int network_get_ipv4_address(ipv4_address_t *ip)
{
    if (!ip) return -1;
    *ip = g_ip;
    return 0;
}

int network_set_ipv4_address(const ipv4_address_t *ip)
{
    if (!ip) return -1;
    g_ip = *ip;
    /* TODO: no ARP yet, so this address isn't announced or checked
       for conflicts — it's just a local record until ARP exists */
    return 0;
}

int network_send_frame(const void *data, uint32_t len)
{
    if (!g_initialized || !g_dev || !g_dev->send) return -1;

    int ret = g_dev->send(g_dev, (const uint8_t *)data, len);
    if (ret == 0) g_frames_sent++;
    return ret;
}

int network_receive_frame(void *buf, uint32_t max_len)
{
    if (!g_initialized || !g_dev || !g_dev->recv) return -1;

    int n = g_dev->recv(g_dev, (uint8_t *)buf, max_len);
    if (n > 0) g_frames_received++;
    return n;
}

/* Вызывается из IRQ-обработчика драйвера NIC при приёме кадра.
   Пока просто считает статистику — обработка (ARP/IP разбор)
   добавится, когда появится сетевой стек выше этого слоя. */
void network_on_frame_received(void)
{
    g_frames_received++;
}

int network_get_frames_sent(void)
{
    return (int)g_frames_sent;
}

int network_get_frames_received(void)
{
    return (int)g_frames_received;
}