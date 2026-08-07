// Copyright (c) 2026 Xetch (https://github.com/xetch-mirror)
// This software is released under the GNU General Public License v2.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include <stdbool.h>

/* Порты контроллера PS/2 (8042) */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_CMD_PORT     0x64

/* Биты регистра состояния (status register) */
#define PS2_STATUS_OUTPUT_FULL   0x01  /* можно читать из data port */
#define PS2_STATUS_INPUT_FULL    0x02  /* нельзя писать, буфер занят */
#define PS2_STATUS_SYSTEM_FLAG   0x04
#define PS2_STATUS_CMD_DATA      0x08  /* 0 = данные, 1 = команда */
#define PS2_STATUS_TIMEOUT_ERR   0x40
#define PS2_STATUS_PARITY_ERR    0x80

/* Команды контроллера */
#define PS2_CMD_READ_CONFIG      0x20
#define PS2_CMD_WRITE_CONFIG     0x60
#define PS2_CMD_DISABLE_PORT2    0xA7
#define PS2_CMD_ENABLE_PORT2     0xA8
#define PS2_CMD_TEST_PORT2       0xA9
#define PS2_CMD_TEST_CONTROLLER  0xAA
#define PS2_CMD_TEST_PORT1       0xAB
#define PS2_CMD_DISABLE_PORT1    0xAD
#define PS2_CMD_ENABLE_PORT1     0xAE
#define PS2_CMD_WRITE_PORT2      0xD4

/* Биты байта конфигурации (config byte) */
#define PS2_CFG_PORT1_IRQ        0x01
#define PS2_CFG_PORT2_IRQ        0x02
#define PS2_CFG_PORT1_CLOCK_DIS  0x10
#define PS2_CFG_PORT2_CLOCK_DIS  0x20
#define PS2_CFG_PORT1_TRANSLATE  0x40

/* Инициализация контроллера: отключение портов, самотест, включение */
bool ps2_init(void);

/* Низкоуровневый обмен с контроллером */
void ps2_wait_input_clear(void);   /* ждать, пока можно писать в data/cmd port */
void ps2_wait_output_full(void);   /* ждать, пока есть что читать из data port */

uint8_t ps2_read_data(void);
void    ps2_write_data(uint8_t val);
void    ps2_send_command(uint8_t cmd);

uint8_t ps2_read_config(void);
void    ps2_write_config(uint8_t cfg);

bool ps2_port1_enabled(void);
bool ps2_port2_enabled(void);

#endif