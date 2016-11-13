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


#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <stdbool.h>
#include "ch.h"
#include "STN1110.h"

enum STN1110_error {
    STN1110_ERROR_NONE,
    STN1110_ERROR_STOPPED,
    STN1110_ERROR_NO_DATA,
    STN1110_ERROR_BUS_INIT
};

void reset_system(void);

void set_system_initialized(bool initialized);
bool get_system_initialized(void);

uint32_t get_pid_poll_delay(void);
void set_pid_poll_delay(uint32_t delay);
void stretch_pid_poll_delay(void);
void reset_pid_poll_delay(void);

void set_pid_request_active(bool active);
bool get_pid_request_active(void);

systime_t get_last_pid_request_time(void);
bool is_pid_request_timeout(systime_t timeout);

systime_t get_obdii_request_timeout(void);
void set_obdii_request_timeout(systime_t timeout);

void set_stn1110_error(enum STN1110_error error);
enum STN1110_error get_stn1110_error(void);

void set_detected_protocol(enum obdii_protocol);
enum obdii_protocol get_detected_protocol(void);

uint32_t get_nodata_error_count(void);
void reset_nodata_error_count(void);
void increment_nodata_error_count(void);

uint32_t get_obdii_timeout_count(void);
void reset_obdii_timeout_count(void);
void increment_obdii_timeout_count(void);

void mark_stn1110_tx(void);
uint32_t mark_stn1110_rx(void);
uint32_t get_stn1110_latency(void);

void broadcast_stats(void);

void check_system_state(void);

#endif /* SYSTEM_H_ */
