/*
 * STN1110.h
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */

#ifndef STN1110_H_
#define STN1110_H_
#include "ch.h"
#include "hal.h"

void stn1110_reset(uint8_t protocol);

void stn1110_worker(void);

#endif /* STN1110_H_ */
