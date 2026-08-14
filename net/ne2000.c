/* NE2000 — самый простой сетевой драйвер: PIO-регистры, кольцевой
   буфер на самой карте (страницы по 256 байт), без DMA/дескрипторов
   как у e1000. Хватает для одного устройства за раз. */

#include "ne2000.h"
#include "kio_ports.h"
#include "sys_io.h"
#include "memory.h"

#define NE_CMD      0x00
#define NE_PSTART   0x01
#define NE_PSTOP    0x02
#define NE_BNRY     0x03
#define NE_TPSR     0x04
#define NE_TBCR0    0x05
#define NE_TBCR1    0x06
#define NE_ISR      0x07
#define NE_RSAR0    0x08
#define NE_RSAR1    0x09
#define NE_RBCR0    0x0A
#define NE_RBCR1    0x0B
#define NE_RCR      0x0C
#define NE_TCR      0x0D
#define NE_DCR      0x0E
#define NE_IMR      0x0F
#define NE_DATA     0x10
#define NE_RESET    0x1F

#define NE_PAGE_SIZE   256
#define NE_RX_START    0x46  /* page 0x46..0x60 = RX ring */
#define NE_RX_STOP     0x60
#define NE_TX_START    0x40  /* page 0x40..0x46 = TX buffer */

static uint16_t g_io_base = 0;
static uint8_t  g_mac[6];
static netdev_t g_dev;

static uint8_t ne_in(uint8_t reg)  { return inb(g_io_base + reg); }
static void    ne_out(uint8_t reg, uint8_t val) { outb(g_io_base + reg, val); }

static int ne2000_send(netdev_t *dev, const uint8_t *frame, uint32_t len);
static int ne2000_recv(netdev_t *dev, uint8_t *buf, uint32_t max_len);

int ne2000_probe(uint16_t io_base)
{
    g_io_base = io_base;

    /* сброс: чтение RESET и запись его же значения обратно */
    uint8_t reset_val = inb(io_base + NE_RESET);
    outb(io_base + NE_RESET, reset_val);

    int timeout = 10000;
    while (!(ne_in(NE_ISR) & 0x80) && timeout--) { }
    if (timeout <= 0) {
        io_print("[ne2000] reset timeout, card not found\n");
        return -1;
    }
    ne_out(NE_ISR, 0xFF); /* clear reset flag */

    /* остановить карту перед настройкой */
    ne_out(NE_CMD, 0x21); /* STOP | no-DMA */
    ne_out(NE_DCR, 0x49); /* word-wide, normal loopback off, FIFO 8 bytes */
    ne_out(NE_RBCR0, 0);
    ne_out(NE_RBCR1, 0);
    ne_out(NE_RCR, 0x20); /* monitor mode while reading MAC */
    ne_out(NE_TCR, 0x02); /* loopback for now, real mode set after MAC read */
    ne_out(NE_PSTART, NE_RX_START);
    ne_out(NE_PSTOP, NE_RX_STOP);
    ne_out(NE_BNRY, NE_RX_START);
    ne_out(NE_ISR, 0xFF);
    ne_out(NE_IMR, 0x00); /* TODO: interrupts not unmasked yet — polling only */

    /* читаем MAC из PROM через remote DMA (страница 0, первые 12 байт
       чередуют MAC-байт/0 в 16-битном режиме) */
    ne_out(NE_CMD, 0x22); /* START | no-DMA */
    ne_out(NE_RSAR0, 0);
    ne_out(NE_RSAR1, 0);
    ne_out(NE_RBCR0, 32);
    ne_out(NE_RBCR1, 0);
    ne_out(NE_CMD, 0x0A); /* START | remote read */

    uint8_t prom[32];
    for (int i = 0; i < 32; i++) prom[i] = inb(io_base + NE_DATA);

    for (int i = 0; i < 6; i++) g_mac[i] = prom[i * 2];

    /* настоящий режим приёма: broadcast + свой unicast */
    ne_out(NE_RCR, 0x04);
    ne_out(NE_TCR, 0x00); /* loopback off */
    ne_out(NE_CMD, 0x22); /* START, page 0 */

    g_dev.name[0] = 'e'; g_dev.name[1] = 't'; g_dev.name[2] = 'h';
    g_dev.name[3] = '0'; g_dev.name[4] = 0;
    for (int i = 0; i < 6; i++) g_dev.mac[i] = g_mac[i];
    g_dev.send = ne2000_send;
    g_dev.recv = ne2000_recv;
    g_dev.irq_handler = 0; /* TODO: no IRQ wired yet, IMR left at 0 */
    g_dev.priv = 0;

    netdev_register(&g_dev);

    io_print("[ne2000] initialized on io_base\n");
    return 0;
}

netdev_t *ne2000_get_device(void)
{
    return &g_dev;
}

static int ne2000_send(netdev_t *dev, const uint8_t *frame, uint32_t len)
{
    (void)dev;
    if (len > NE_PAGE_SIZE * 6) return -1; /* TX buffer is 6 pages */

    /* remote DMA write into TX_START page */
    ne_out(NE_ISR, 0x40); /* clear RDC (remote DMA complete) */
    ne_out(NE_RSAR0, 0);
    ne_out(NE_RSAR1, NE_TX_START);
    ne_out(NE_RBCR0, (uint8_t)(len & 0xFF));
    ne_out(NE_RBCR1, (uint8_t)(len >> 8));
    ne_out(NE_CMD, 0x12); /* START | remote write */

    for (uint32_t i = 0; i < len; i++) outb(g_io_base + NE_DATA, frame[i]);

    int timeout = 10000;
    while (!(ne_in(NE_ISR) & 0x40) && timeout--) { } /* wait RDC */
    ne_out(NE_ISR, 0x40);

    /* kick transmit */
    ne_out(NE_TPSR, NE_TX_START);
    ne_out(NE_TBCR0, (uint8_t)(len & 0xFF));
    ne_out(NE_TBCR1, (uint8_t)(len >> 8));
    ne_out(NE_CMD, 0x26); /* START | TXP | no-DMA */

    return 0;
}

static int ne2000_recv(netdev_t *dev, uint8_t *buf, uint32_t max_len)
{
    (void)dev;

    uint8_t bound = ne_in(NE_BNRY);
    uint8_t curr;
    ne_out(NE_CMD, 0x62); /* page 1, to read CURR */
    curr = ne_in(0x07);   /* CURR register lives at offset 7 on page 1 */
    ne_out(NE_CMD, 0x22); /* back to page 0 */

    if (bound == curr) return 0; /* nothing new */

    /* TODO: this doesn't yet parse the 4-byte NE2000 packet header
       (status, next-page, length) prefixed to each buffered frame —
       real implementation must remote-read that header first to know
       actual length before copying, and must handle ring wraparound
       at NE_RX_STOP. This is a stub showing the polling shape only. */
    (void)max_len;
    (void)buf;

    return -1; /* not implemented yet */
}