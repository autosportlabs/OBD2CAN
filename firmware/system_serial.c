/*
 * serial.c
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
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

void system_serial_init(void)
{
    /* Initialize connection to STN1110 on SD2
     */
    static SerialConfig stn_uart_cfg;
    stn_uart_cfg.speed=9600;

    /* USART2 TX.       */
    palSetPadMode(GPIOA, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(1));
    /* USART2 RX.       */
    palSetPadMode(GPIOA, 3, PAL_STM32_MODE_ALTERNATE | PAL_STM32_PUPDR_PULLUP | PAL_STM32_ALTERNATE(1));
    sdStart(&SD2, &stn_uart_cfg);

    /*
     * Activates the serial driver 1 (debug port) using the driver default configuration.
     * PA9 and PA10 are routed to USART1.
     */
    static SerialConfig debug_uart_cfg;
    debug_uart_cfg.speed=9600;
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */
    sdStart(&SD1, &debug_uart_cfg);
}
