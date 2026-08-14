void net_init(uint16_t io_base) {
    (void)io_base;
    e1000_mmio_base = (uint8_t *)pci_get_bar0(INTEL_VEND, E1000_DEV);

    if (!e1000_mmio_base) {
        syslogd_log(LOG_ERR, "net_d", "E1000 MMIO lookup failed");
        io_print("[net_d] error: E1000 NIC not found on PCI bus\n");
        return;
    }

    /* GIVE IT SPACE — read-modify-write, don't clobber other bits */
    uint16_t pci_cmd = pci_read_cmd(INTEL_VEND, E1000_DEV);
    pci_write_cmd(INTEL_VEND, E1000_DEV, pci_cmd | 0x06); /* Bus Master | Memory Space */

    /* BIG RESET */
    uint32_t ctrl = e1000_read(REG_CTRL);
    e1000_write(REG_CTRL, ctrl | (1 << 26));

    /* wait for reset to actually take — RST self-clears when done */
    int reset_timeout = 100000;
    while ((e1000_read(REG_CTRL) & (1 << 26)) && reset_timeout--) { }
    if (reset_timeout <= 0) {
        io_print("[net_d] warning: reset did not clear in time\n");
    }

    /* clear the Multicast Table Array — 128 entries, must be zeroed
       before RX is enabled or stale state from a previous OS/BIOS
       can leak through */
    for (int i = 0; i < 128; i++) {
        e1000_write(REG_MTA + (i * 4), 0);
    }

    /* read MAC from EEPROM and program RAL0/RAH0 so the NIC
       recognizes its own address on RX */
    uint8_t mac[6];
    for (int i = 0; i < 3; i++) {
        uint32_t word = e1000_eeprom_read(i); /* TODO: assumes an
            e1000_eeprom_read() exists elsewhere reading via EERD
            register; if not yet written, this call needs a real
            implementation (write address+start bit to REG_EERD,
            poll done bit, read data field) before this works */
        mac[i * 2]     = word & 0xFF;
        mac[i * 2 + 1] = (word >> 8) & 0xFF;
    }
    uint32_t ral = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    uint32_t rah = mac[4] | (mac[5] << 8) | (1u << 31); /* AV = address valid */
    e1000_write(REG_RAL0, ral);
    e1000_write(REG_RAH0, rah);

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

    /* unmask RX interrupt causes so this stops being polling-only —
       RXT0 (RX timer, bit 7) and RXDMT0 (RX descriptor min threshold,
       bit 4) cover the common "a frame arrived" cases */
    e1000_write(REG_IMS, (1 << 7) | (1 << 4));

    /* confirm link actually came up before assuming TX/RX will work */
    int link_timeout = 100000;
    uint32_t status;
    do {
        status = e1000_read(REG_STATUS);
        link_timeout--;
    } while (!(status & (1 << 1)) && link_timeout > 0); /* LU = bit 1 */

    if (status & (1 << 1)) {
        io_print("[net_d] link up\n");
    } else {
        io_print("[net_d] warning: link not detected\n");
    }

    syslogd_log(LOG_INFO, "net_d", "initialized");
    io_print("[net_d] network daemon ready\n");
}