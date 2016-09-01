/*
 * serial.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef SERIAL_H_
#define SERIAL_H_
#include "ch.h"
#include "hal.h"

#define SD1_BAUD 115200
#define SD2_BAUD 9600

/*
 * Read a line from the specified serial connection into the specified
 * buffer buf with a length of buf_len
 * This call blocks until a \r or buf_len is reached.
 */
size_t serial_getline(SerialDriver *sdp, uint8_t *buf, size_t buf_len);

/*
 * Initialize serial and usart subsystems
 */

void system_serial_init_SD1(uint32_t speed);
void system_serial_init_SD2(uint32_t speed);
void system_serial_init(void);

#endif /* SERIAL_H_ */
