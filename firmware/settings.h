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

#ifndef SETTINGS_H_
#define SETTINGS_H_

/* The CAN IDs and control codes
 * for our control and diagnostic messages */
#define OBD2CAN_CTRL_ID 62344
#define OBD2CAN_STATS_ID 62345
#define CTRL_CMD_CONFIGURE 0x01

/*SAE OBDII constants */
#define OBDII_PID_REQUEST 0x7df
#define OBDII_PID_RESPONSE 0x7e8
#define CUSTOM_MODE_SHOW_CURRENT_DATA   0x41
#define MAX_CAN_MESSAGE_SIZE 8

/* The starting PID poll delay and maximum
 * poll delay. Poll delay can be dynamically
 * stretched
 */
#define OBDII_MIN_PID_POLL_DELAY 50
#define OBDII_MAX_PID_POLL_DELAY 250
#define OBDII_PID_POLL_DELAY_STRETCH 10

/* How long we delay if we get a PID error response */
#define OBDII_PID_ERROR_DELAY 200

/* The timeout value while we wait
 * for an available CAN transmission slot */
#define CAN_TRANSMIT_TIMEOUT 100

/* How long we wait before timing out on a CAN PID request */
#define OBDII_INITIAL_TIMEOUT 5000
#define OBDII_RUNTIME_TIMEOUT 300

/* Error thresholds before we issue a reboot */
#define MAX_NODATA_ERRORS 20
#define MAX_OBDII_TIMEOUTS 10

/* how long we wait before resetting the system */
#define SYSTEM_RESET_DELAY 10

/* Baud rates for STN1110 co-processor */
#define STN1110_INITIAL_BAUD_RATE 9600
#define STN1110_RUNTIME_BAUD_RATE 230400

#endif /* SETTINGS_H_ */
