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
#include "logging.h"

static enum logging_levels logging_level = logging_level_trace;

void set_logging_level(enum logging_levels level)
{
    if (level > logging_level_trace)
        return;
    logging_level = level;
}

enum logging_levels get_logging_level(void)
{
    return logging_level;
}

static void _log_trace_can_frame(uint8_t dlc, uint8_t ide, uint32_t sid, uint32_t eid, uint8_t * data_bytes)
{
    uint32_t can_id = ide == CAN_IDE_EXT ? eid : sid;
    log_trace_b(" ID(%i) ", can_id);

    size_t i;
    for (i = 0; i < dlc; i++) {
        log_trace_b("%02X ", data_bytes[i]);
    }
}
void log_CAN_rx_message(char* log_pfx, CANRxFrame * can_frame)
{
    if (get_logging_level() < logging_level_info)
        return;

    log_info(log_pfx);
    log_info_b("CAN Rx");

    if (get_logging_level() >= logging_level_trace) {
        _log_trace_can_frame(can_frame->DLC, can_frame->IDE, can_frame->SID, can_frame->EID, can_frame->data8);
    }
    log_info_b("\r\n");
}


void log_CAN_tx_message(char *log_pfx, CANTxFrame * can_frame)
{
    if (get_logging_level() < logging_level_info)
        return;

    log_info(log_pfx);
    log_info_b("CAN Tx");

    if (get_logging_level() >= logging_level_trace) {
        _log_trace_can_frame(can_frame->DLC, can_frame->IDE, can_frame->SID, can_frame->EID, can_frame->data8);
    }
    log_info_b("\r\n");
}
