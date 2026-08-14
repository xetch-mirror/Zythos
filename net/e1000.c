void net_init(uint16_t io_base) {
    (void)io_base;
    e1000_mmio_base = (uint8_t *)pci_get_bar0(INTEL_VEND, E1000_DEV);

    if (!e1000_mmio_base) {
        syslogd_log(LOG_ERR, "net_d", "E1000 MMIO lookup failed");
        io_print("[net_d] error: E1000 NIC not found on PCI bus\n");
        return;
    }

    // GIVE IT SPACE
    pci_write_cmd(INTEL_VEND, E1000_DEV, 0x06);

    // BIG RESET
    uint32_t ctrl = e1000_read(REG_CTRL);
    e1000_write(REG_CTRL, ctrl | (1 << 26));

    // RX
    for (int i = 0; i < NUM_RX_DESC; i++) {
        rx_descs[i].addr = (uint64_t)(uintptr_t)rx_buffers[i];
        rx_descs[i].status = 0;
    }
    e1000_write(REG_RDBAL, (uint32_t)(uintptr_t)rx_descs);
    e1000_write(REG_RDBAH, 0);
    e1000_write(REG_RDLEN, NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    e1000_write(REG_RDH, 0);
    e1000_write(REG_RDT, NUM_RX_DESC - 1);

    // RX
    uint32_t rctl = e1000_read(REG_RCTL);
    e1000_write(REG_RCTL, rctl | (1 << 1) | (1 << 3) | (1 << 15) | (1 << 26));

    // Setup TX Descriptors
    for (int i = 0; i < NUM_TX_DESC; i++) {
        tx_descs[i].addr = (uint64_t)(uintptr_t)tx_buffers[i];
        tx_descs[i].cmd = 0;
        tx_descs[i].status = 1;
    }
    e1000_write(REG_TDBAL, (uint32_t)(uintptr_t)tx_descs);
    e1000_write(REG_TDBAH, 0);
    e1000_write(REG_TDLEN, NUM_TX_DESC * sizeof(e1000_tx_desc_t));
    e1000_write(REG_TDH, 0);
    e1000_write(REG_TDT, 0);

    // OK
    uint32_t tctl = e1000_read(REG_TCTL);
    e1000_write(REG_TCTL, tctl | (1 << 1) | (1 << 3));

    syslogd_log(LOG_INFO, "net_d", "initialized");
    io_print("[net_d] network daemon ready\n");
}