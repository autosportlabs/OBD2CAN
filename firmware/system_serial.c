/*
 * OBD2CAN firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "system_serial.h"


size_t serial_getline(SerialDriver *sdp, uint8_t *buf, size_t buf_len) {
  size_t n;
  uint8_t c;

  n = 0;
  do {
    c = sdGet(sdp);
    *buf++ = c;
    n++;
  } while (c != '\r' && n < buf_len - 1);
  *buf = 0;
  return n;
}

/*
 * Initialize connection for SD1 (logger)
 */
void system_serial_init_SD1(uint32_t speed)
{
    /*
     * Activates the serial driver 1 (debug port) using the driver default configuration.
     * PA9 and PA10 are routed to USART1.
     */
    static SerialConfig uart_cfg;
    uart_cfg.speed=speed;
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */
    sdStart(&SD1, &uart_cfg);
}

/*
 * Initialize connection for SD2 (STN1110)
 */
void system_serial_init_SD2(uint32_t speed)
{
    static SerialConfig uart_cfg;
    uart_cfg.speed=speed;

    /* USART2 TX.       */
    palSetPadMode(GPIOA, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(1));
    /* USART2 RX.       */
    palSetPadMode(GPIOA, 3, PAL_STM32_MODE_ALTERNATE | PAL_STM32_PUPDR_PULLUP | PAL_STM32_ALTERNATE(1));
    sdStart(&SD2, &uart_cfg);
}

void system_serial_init()
{
    system_serial_init_SD1(SD1_BAUD);
    system_serial_init_SD2(SD2_BAUD);
}
