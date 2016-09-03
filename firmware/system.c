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

#include "system.h"
#include "settings.h"
#include "ch.h"
#include "hal.h"

static bool system_initialized = false;
static bool pid_request_active = false;
static systime_t pid_request_time = 0;
static systime_t obdii_request_timeout = OBDII_INITIAL_TIMEOUT;

/*metrics information */
static systime_t stn1110_message_rx_timestamp = 0;
static systime_t stn1110_message_tx_timestamp = 0;
static uint32_t stn1110_latency_ms = 0;

/*Error statistics */
static enum STN1110_error stn1110_last_error = STN1110_ERROR_NONE;

void set_system_initialized(bool initialized)
{
	system_initialized = initialized;
}

bool get_system_initialized(void)
{
	return system_initialized;
}

void set_pid_request_active(bool active)
{
	pid_request_active = active;
	pid_request_time = active ? chVTGetSystemTime() : 0;
}

bool get_pid_request_active(void)
{
	return pid_request_active;
}

systime_t get_last_pid_request_time(void)
{
    return pid_request_time;
}

bool is_pid_request_timeout(systime_t timeout)
{
    return  pid_request_active &&
            pid_request_time > 0 &&
            chVTTimeElapsedSinceX(pid_request_time) > MS2ST(timeout);
}

systime_t get_obdii_request_timeout(void)
{
    return obdii_request_timeout;
}

void set_obdii_request_timeout(systime_t timeout)
{
    obdii_request_timeout = timeout;
}

void set_stn1110_error(enum STN1110_error error)
{
    stn1110_last_error = error;
}

enum STN1110_error get_stn1110_error(void)
{
    return stn1110_last_error;
}

void mark_stn1110_tx(void)
{
    stn1110_message_tx_timestamp = chVTGetSystemTime();
}

uint32_t mark_stn1110_rx(void)
{
    stn1110_message_rx_timestamp = chVTGetSystemTime();
    stn1110_latency_ms = ST2MS(chVTTimeElapsedSinceX(stn1110_message_tx_timestamp));
    return stn1110_latency_ms;
}

uint32_t get_stn1110_latency(void){
    return stn1110_latency_ms;
}

void broadcast_stats(void){
	CANTxFrame can_stats;
	can_stats.IDE = CAN_IDE_EXT;
	can_stats.EID = OBD2CAN_STATS_ID;
	can_stats.RTR = CAN_RTR_DATA;
	can_stats.DLC = 8;
	can_stats.data16[0] = get_stn1110_latency();
	can_stats.data8[2] = get_stn1110_error();
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &can_stats, MS2ST(CAN_TRANSMIT_TIMEOUT));
}
