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
#include "system_CAN.h"

#define _LOG_PFX "SYS:         "

/* Flag to indicate if system is initialized
 * and ready for normal operation */
static bool system_initialized = false;

/* Flag to indicate if a PID request is currently active */
static bool pid_request_active = false;

/* Counter for error conditions */
static uint32_t nodata_error_count = 0;
static uint32_t obdii_timeout_count = 0;

/* The timeout value for our OBDII request */
static systime_t obdii_request_timeout = OBDII_INITIAL_TIMEOUT;

/* Built-in delay for our processing OBDII PID requests */
static uint32_t pid_poll_delay = OBDII_MIN_PID_POLL_DELAY;

/*metrics information */

/* Detected OBDII protocol */
enum obdii_protocol detected_protocol = obdii_protocol_auto;

/* Timestamps for whenw we receive and send STN1110 messages */
static systime_t stn1110_message_rx_timestamp = 0;
static systime_t stn1110_message_tx_timestamp = 0;

/* Calculated latency for STN1110 request / response */
static uint32_t stn1110_latency_ms = 0;

/* Time when we last requested a PID via CAN */
static systime_t pid_request_time = 0;

/*Error statistics */
static enum STN1110_error stn1110_last_error = STN1110_ERROR_NONE;

/* Get / Set detected OBDII protocol */
void set_detected_protocol(enum obdii_protocol protocol)
{
    detected_protocol = protocol;
}

enum obdii_protocol get_detected_protocol(void)
{
    return detected_protocol;
}

/* Get / Set system initialized flag */
void set_system_initialized(bool initialized)
{
    system_initialized = initialized;
}

bool get_system_initialized(void)
{
    return system_initialized;
}

/* Get / Set for delay in-between PID requests */
uint32_t get_pid_poll_delay(void)
{
    return pid_poll_delay;
}

void set_pid_poll_delay(uint32_t delay)
{
    pid_poll_delay = delay;
}

/* Stretch the PID poll delay by a pre-determined amount.
 * Needed to auto-tune the system for maximum performance
 * based on the current OBDII protocol
 */
void stretch_pid_poll_delay(void)
{
    if (pid_poll_delay < OBDII_MAX_PID_POLL_DELAY) {
        pid_poll_delay += OBDII_PID_POLL_DELAY_STRETCH;
        log_info(_LOG_PFX "Stretching PID poll delay by %ims to %ims\r\n", OBDII_PID_POLL_DELAY_STRETCH, pid_poll_delay);
    } else {
        log_info(_LOG_PFX "Max PID poll delay reached: %ims\r\n", pid_poll_delay);
    }
}

/* Reset our PID poll delay to the initial value */
void reset_pid_poll_delay(void)
{
    pid_poll_delay = OBDII_MIN_PID_POLL_DELAY;
    log_info(_LOG_PFX "Reset PID poll delay: %ims\r\n", pid_poll_delay);

}

/* Get / Set PID request currently active flag */
void set_pid_request_active(bool active)
{
    pid_request_active = active;
    pid_request_time = active ? chVTGetSystemTime() : 0;
}

bool get_pid_request_active(void)
{
    return pid_request_active;
}

/* Get the last timestamp when a PID was requested. */
systime_t get_last_pid_request_time(void)
{
    return pid_request_time;
}

/* Returns true if our current PID request has timed out */
bool is_pid_request_timeout(systime_t timeout)
{
    return  pid_request_active &&
            pid_request_time > 0 &&
            chVTTimeElapsedSinceX(pid_request_time) > MS2ST(timeout);
}

/* Get / Set the OBDII request timeout value */
systime_t get_obdii_request_timeout(void)
{
    return obdii_request_timeout;
}

void set_obdii_request_timeout(systime_t timeout)
{
    obdii_request_timeout = timeout;
}

/* Get / Set the current STN1110 error code */
void set_stn1110_error(enum STN1110_error error)
{
    stn1110_last_error = error;
}

enum STN1110_error get_stn1110_error(void)
{
    return stn1110_last_error;
}

/* Mark the time when we last set a PID request to the STN1110 */
void mark_stn1110_tx(void)
{
    stn1110_message_tx_timestamp = chVTGetSystemTime();
}

/* Mark the time when we last received a PID response from the STN1110, error or success */
uint32_t mark_stn1110_rx(void)
{
    stn1110_message_rx_timestamp = chVTGetSystemTime();
    stn1110_latency_ms = ST2MS(chVTTimeElapsedSinceX(stn1110_message_tx_timestamp));
    return stn1110_latency_ms;
}

/* Get the current calculated STN1110 PID request round trip latency */
uint32_t get_stn1110_latency(void)
{
    return stn1110_latency_ms;
}

/* Broadcast some current stats */
void broadcast_stats(void)
{
    CANTxFrame can_stats;
    prepare_can_tx_message(&can_stats, CAN_IDE_EXT, OBD2CAN_STATS_ID);
    can_stats.data8[0] = get_detected_protocol();

    /* byte 1 - current STN1110 error code */
    can_stats.data8[1] = get_stn1110_error();

    /* byte 2-3 - stn1100 PID request latency */
    can_stats.data16[1] = (uint16_t)get_stn1110_latency();

    /* byte 4 - reserved */
    can_stats.data8[4] = 0;

    /* byte 5-7 send version information */
    can_stats.data8[5] = MAJOR_VER;
    can_stats.data8[6] = MINOR_VER;
    can_stats.data8[7] = PATCH_VER;
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &can_stats, MS2ST(CAN_TRANSMIT_TIMEOUT));
    log_trace(_LOG_PFX "Broadcasting stats\r\n");
}

/* perform a soft reset of this processor */
void reset_system(void)
{
    log_info(_LOG_PFX "Resetting System\r\n");
    chThdSleepMilliseconds(SYSTEM_RESET_DELAY);
    NVIC_SystemReset();
}

/* Get the current error count of a failed STN1110 PID request */
uint32_t get_nodata_error_count(void)
{
    return nodata_error_count;
}

/* Reset our error counter */
void reset_nodata_error_count(void)
{
    nodata_error_count = 0;
}

/* Increment our error counter */
void increment_nodata_error_count(void)
{
    nodata_error_count++;
}

/* Get the current number of OBDII PID request timeouts */
uint32_t get_obdii_timeout_count(void)
{
    return obdii_timeout_count;
}

/* Reset our OBDII PID request timeout counter */
void reset_obdii_timeout_count(void)
{
    obdii_timeout_count = 0;
}

/* Increment our OBDII request timeout counter */
void increment_obdii_timeout_count(void)
{
    obdii_timeout_count++;
}

/* Check if we're in a state where we need to reset the system */
void check_system_state(void)
{
    if (get_nodata_error_count() > MAX_NODATA_ERRORS) {
        log_info(_LOG_PFX "Too many no response errors\r\n");
        /* Nuclear option */
        reset_system();
    }
}

