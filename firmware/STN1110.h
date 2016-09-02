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

#ifndef STN1110_H_
#define STN1110_H_
#include "ch.h"
#include "hal.h"

void stn1110_reset(uint8_t protocol);

void stn1110_worker(void);

enum obdii_protocol {
    obdii_protocol_auto,
    obdii_protocol_j1850_pwm,
    obdii_protocol_j1850_vpw,
    obdii_protocol_9141_2,
    obdii_protocol_iso14230_4_kwp_5baud,
    obdii_protocol_iso14230_4_fast_init
};
#endif /* STN1110_H_ */
