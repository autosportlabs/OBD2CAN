/*
 * CAN.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef CAN_H_
#define CAN_H_
#include "ch.h"
#include "hal.h"

void system_can_init(void);
void can_worker(void);
void dispatch_can_rx(CANRxFrame *rx_msg);

#endif /* CAN_H_ */
