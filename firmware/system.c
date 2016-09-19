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
#include "logging.h"

#define _LOG_PFX "SYS:         "

static bool system_initialized = false;
static bool pid_request_active = false;
static uint32_t nodata_error_count = 0;
static uint32_t obdii_timeout_count = 0;
static systime_t pid_request_time = 0;
static systime_t obdii_request_timeout = OBDII_INITIAL_TIMEOUT;
static uint32_t pid_poll_delay = OBDII_MIN_PID_POLL_DELAY;

/*metrics information */
enum obdii_protocol detected_protocol = obdii_protocol_auto;
static systime_t stn1110_message_rx_timestamp = 0;
static systime_t stn1110_message_tx_timestamp = 0;
static uint32_t stn1110_latency_ms = 0;

/*Error statistics */
static enum STN1110_error stn1110_last_error = STN1110_ERROR_NONE;

void set_detected_protocol(enum obdii_protocol protocol)
{
    detected_protocol = protocol;
}
enum obdii_protocol get_detected_protocol(void)
{
    return detected_protocol;
}

void set_system_initialized(bool initialized)
{
	system_initialized = initialized;
}

bool get_system_initialized(void)
{
	return system_initialized;
}

uint32_t get_pid_poll_delay(void)
{
    return pid_poll_delay;
}

void set_pid_poll_delay(uint32_t delay)
{
    pid_poll_delay = delay;
}

void stretch_pid_poll_delay(void)
{
    if (pid_poll_delay < OBDII_MAX_PID_POLL_DELAY){
        pid_poll_delay += OBDII_PID_POLL_DELAY_STRETCH;
        log_info(_LOG_PFX "Stretching PID poll delay by %ims to %ims\r\n", OBDII_PID_POLL_DELAY_STRETCH, pid_poll_delay);
    }
    else{
        log_info(_LOG_PFX "Max PID poll delay reached: %ims\r\n", pid_poll_delay);
    }
}

void reset_pid_poll_delay(void)
{
    pid_poll_delay = OBDII_MIN_PID_POLL_DELAY;
    log_info(_LOG_PFX "Reset PID poll delay: %ims\r\n", pid_poll_delay);

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
	can_stats.data8[0] = get_detected_protocol();
    can_stats.data8[1] = get_stn1110_error();
	can_stats.data16[1] = (uint16_t)get_stn1110_latency();
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &can_stats, MS2ST(CAN_TRANSMIT_TIMEOUT));
}

void reset_system(void)
{
    log_info(_LOG_PFX "Resetting System\r\n");
    chThdSleepMilliseconds(SYSTEM_RESET_DELAY);
    NVIC_SystemReset();
}

uint32_t get_nodata_error_count(void)
{
    return nodata_error_count;
}

void reset_nodata_error_count(void)
{
    nodata_error_count = 0;
}

void increment_nodata_error_count(void)
{
    nodata_error_count++;
}

uint32_t get_obdii_timeout_count(void)
{
    return obdii_timeout_count;
}

void reset_obdii_timeout_count(void)
{
    obdii_timeout_count = 0;
}

void increment_obdii_timeout_count(void)
{
    obdii_timeout_count++;
}
