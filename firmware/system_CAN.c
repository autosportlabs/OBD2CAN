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

#include "system_CAN.h"
#include "logging.h"
#include "system_serial.h"
#include "settings.h"
#include "system.h"
#include "STN1110.h"

#define _LOG_PFX "SYS_CAN:     "

/*
 * 250KBaud, automatic wakeup, Automatic Bus-off management, Transmit FIFO priority
 *
 * 500K baud; 36MHz clock
 */
static const CANConfig cancfg = {
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP | CAN_MCR_NART,
    CAN_BTR_SJW(1) |
    CAN_BTR_TS1(11) | CAN_BTR_TS2(2) | CAN_BTR_BRP(5)
};

/* Initialize our CAN peripheral */
void system_can_init(void)
{
    /* CAN RX.       */
    palSetPadMode(GPIOA, 11, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));
    /* CAN TX.       */
    palSetPadMode(GPIOA, 12, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));

    /* Activates the CAN driver */
    canStart(&CAND1, &cancfg);
}

/* Process a command and configure message */
static void _process_configure_cmd(CANRxFrame *rx_msg)
{
    enum obdii_protocol protocol = DEFAULT_OBDII_PROTOCOL;
    bool should_reset = false;
    enum obdii_adaptive_timing adaptive_timing = DEFAULT_OBDII_ADAPTIVE_TIMING;
    uint8_t obdii_timeout = DEFAULT_OBDII_TIMEOUT;

    switch (rx_msg->DLC) {
    case 6:
        obdii_timeout = rx_msg->data8[5];
    case 5:
        adaptive_timing = rx_msg->data8[4];
    case 4:
        protocol = rx_msg->data8[3];
    case 3:
        should_reset = rx_msg->data8[2] != 0;
    case 2:
        set_logging_level((enum logging_levels)rx_msg->data8[1]);
    }

    if (should_reset) {
        stn1110_reset(protocol, adaptive_timing, obdii_timeout);
    }
}

/* Process and dispatch an incoming control message */
static void _dispatch_ctrl_rx(CANRxFrame *rx_msg)
{
    uint8_t dlc = rx_msg->DLC;
    if (dlc < 2) {
        log_info(_LOG_PFX "Invalid control msg length: %i\r\n", dlc);
        return;
    }

    uint8_t ctrl_cmd = rx_msg->data8[0];
    switch(ctrl_cmd) {
    case CTRL_CMD_CONFIGURE: {
        _process_configure_cmd(rx_msg);
        break;
    }
    default:
        log_info(_LOG_PFX "Unknown control message command: %i\r\n", ctrl_cmd);
        break;
    }
}

/* Process an incoming OBDII PID request */
static void _process_pid_request(CANRxFrame *rx_msg)
{

    if (!get_system_initialized()) {
        log_trace(_LOG_PFX "Ignoring PID request: system initializing\r\n");
        return;
    }

    uint8_t data_byte_count = rx_msg->data8[0];
    if (data_byte_count > MAX_CAN_MESSAGE_SIZE - 1) {
        log_info(_LOG_PFX "Invalid PID request; max data bytes %i exceeded %i\r\n", data_byte_count, MAX_CAN_MESSAGE_SIZE - 1);
        return;
    }

    send_stn1110_pid_request(rx_msg->data8 + 1, data_byte_count);
}

/*
 * Dispatch an incoming CAN message
 */
void dispatch_can_rx(CANRxFrame *rx_msg)
{
    uint8_t can_id_type = rx_msg->IDE;

    switch (can_id_type) {
    case CAN_IDE_EXT:
        /* Process Extended CAN IDs */
    {
        switch (rx_msg->EID) {
        case OBD2CAN_CTRL_ID:
            _dispatch_ctrl_rx(rx_msg);
            break;
        }
        break;
    }
    break;

    case CAN_IDE_STD:
        /* Process Standard CAN IDs */
    {
        switch (rx_msg->SID) {
        case OBDII_PID_REQUEST:
            _process_pid_request(rx_msg);
            break;
        }
    }
    break;
    }
}

/* Main worker for receiving CAN messages */
void can_worker(void)
{
    log_info(_LOG_PFX "CAN worker start\r\n");
    event_listener_t el;
    CANRxFrame rx_msg;
    chRegSetThreadName("CAN receiver");
    chEvtRegister(&CAND1.rxfull_event, &el, 0);
    while(!chThdShouldTerminateX()) {
        if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
            continue;
        while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rx_msg, TIME_IMMEDIATE) == MSG_OK) {
            /* Process message.*/
            log_CAN_rx_message(_LOG_PFX, &rx_msg);
            dispatch_can_rx(&rx_msg);
        }
    }
    chEvtUnregister(&CAND1.rxfull_event, &el);
}

/* Prepare a CAN message with the specified CAN ID and type */
void prepare_can_tx_message(CANTxFrame *tx_frame, uint8_t can_id_type, uint32_t can_id)
{
    tx_frame->IDE = can_id_type;
    if (can_id_type == CAN_IDE_EXT) {
        tx_frame->EID = can_id;
    } else {
        tx_frame->SID = can_id;
    }
    tx_frame->RTR = CAN_RTR_DATA;
    tx_frame->DLC = 8;
    tx_frame->data8[0] = 0x55;
    tx_frame->data8[1] = 0x55;
    tx_frame->data8[2] = 0x55;
    tx_frame->data8[3] = 0x55;
    tx_frame->data8[4] = 0x55;
    tx_frame->data8[5] = 0x55;
    tx_frame->data8[6] = 0x55;
    tx_frame->data8[7] = 0x55;

}
