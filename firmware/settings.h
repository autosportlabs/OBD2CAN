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

#define OBD2CAN_CTRL_ID 62344
#define OBD2CAN_STATS_ID 62345
#define OBDII_MIN_PID_POLL_DELAY 50
#define OBDII_MAX_PID_POLL_DELAY 250
#define OBDII_PID_POLL_DELAY_STRETCH 10
#define OBDII_PID_ERROR_DELAY 200
#define CAN_TRANSMIT_TIMEOUT 100
#define CTRL_CMD_CONFIGURE 0x01
#define OBDII_PID_REQUEST 0x7df
#define OBDII_PID_RESPONSE 0x7e8
#define OBDII_INITIAL_TIMEOUT 5000
#define OBDII_RUNTIME_TIMEOUT 300
#define CUSTOM_MODE_SHOW_CURRENT_DATA   0x41
#define MAX_CAN_MESSAGE_SIZE 8

#endif /* SETTINGS_H_ */
