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

#ifndef LOGGING_H_
#define LOGGING_H_
#include "ch.h"
#include "hal.h"
#include "chprintf.h"


enum logging_levels {
    logging_level_none,
    logging_level_info,
    logging_level_trace
};

#define log_info(msg, ...) if (get_logging_level() >= logging_level_info) {chprintf((BaseSequentialStream *)&SD1, "%i ", ST2MS(chVTGetSystemTimeX())); chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}
#define log_trace(msg, ...) if (get_logging_level() >= logging_level_trace) {chprintf((BaseSequentialStream *)&SD1, "%i ", ST2MS(chVTGetSystemTimeX())); chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}

/* Brief functions */
#define log_info_b(msg, ...) if (get_logging_level() >= logging_level_info) {chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}
#define log_trace_b(msg, ...) if (get_logging_level() >= logging_level_trace) {chprintf((BaseSequentialStream *)&SD1, msg, ##__VA_ARGS__);}

void set_logging_level(enum logging_levels level);

enum logging_levels get_logging_level(void);

void log_CAN_rx_message(char* log_pfx, CANRxFrame * can_frame);

void log_CAN_tx_message(char *log_pfx, CANTxFrame * can_frame);

#endif /* LOGGING_H_ */
