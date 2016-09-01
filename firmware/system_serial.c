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

/*
 * Initialize connection for SD1 (logger)
 */
static void _system_serial_init_SD1(void)
{
    /*
     * Activates the serial driver 1 (debug port) using the driver default configuration.
     * PA9 and PA10 are routed to USART1.
     */
    static SerialConfig uart_cfg;
    uart_cfg.speed=SD1_BAUD;
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */
    sdStart(&SD1, &uart_cfg);
}

/*
 * Initialize connection for SD2 (STN1110)
 */
static void _system_serial_init_SD2(void)
{
    static SerialConfig uart_cfg;
    uart_cfg.speed=9600;

    /* USART2 TX.       */
    palSetPadMode(GPIOA, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(1));
    /* USART2 RX.       */
    palSetPadMode(GPIOA, 3, PAL_STM32_MODE_ALTERNATE | PAL_STM32_PUPDR_PULLUP | PAL_STM32_ALTERNATE(1));
    sdStart(&SD2, &uart_cfg);
}

void system_serial_init(void)
{
    _system_serial_init_SD1();
    _system_serial_init_SD2();
}
