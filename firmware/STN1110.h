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

enum obdii_adaptive_timing {
    obdii_adaptive_timing_disabled,
    obdii_adaptive_timing_normal,
    obdii_adaptive_timing_aggressive
};

enum obdii_protocol {
    obdii_protocol_auto,
    obdii_protocol_j1850_pwm,
    obdii_protocol_j1850_vpw,
    obdii_protocol_9141_2,
    obdii_protocol_iso14230_4_kwp_5baud,
    obdii_protocol_iso14230_4_fast_init
};

#define ADAPTIVE_TIMING_DISABLED 0
#define DEFAULT_OBDII_PROTOCOL obdii_protocol_auto
#define DEFAULT_OBDII_ADAPTIVE_TIMING obdii_adaptive_timing_normal
#define DEFAULT_OBDII_TIMEOUT ADAPTIVE_TIMING_DISABLED

void stn1110_reset(enum obdii_protocol protocol, enum obdii_adaptive_timing adaptive_timing, uint8_t obdii_timeout);
void send_stn1110_pid_request(uint8_t * data, size_t data_len);
void stn1110_worker(void);

void check_voltage_regulator_control(void);
#endif /* STN1110_H_ */
