// netdev.h
typedef struct netdev {
    char name[16];
    uint8_t mac[6];
    int (*send)(struct netdev *dev, const uint8_t *frame, uint32_t len);
    int (*recv)(struct netdev *dev, uint8_t *buf, uint32_t max_len);
    void (*irq_handler)(struct netdev *dev);
    void *priv; /* driver-specific state (ring buffers, io ports, etc) */
} netdev_t;

int netdev_register(netdev_t *dev);
netdev_t *netdev_get_default(void);