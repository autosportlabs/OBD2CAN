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

#define LOG_PFX "SYS_CAN:     "

#define MAX_PID_DATA_BYTES 7

/*
 * 500KBaud, automatic wakeup, Automatic Bus-off management, Transmit FIFO priority
 */
/* Note; TS1 should be 13, probably off b/c internal oscillator. check when switching to HSE */
static const CANConfig cancfg = {
        CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(1) | CAN_BTR_TS2(2) |
  CAN_BTR_TS1(9) | CAN_BTR_BRP(6)
};

void system_can_init(void)
{
    /* CAN RX.       */
    palSetPadMode(GPIOA, 11, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));
    /* CAN TX.       */
    palSetPadMode(GPIOA, 12, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));

    /*
     * Activates the CAN driver
     */
    canStart(&CAND1, &cancfg);
}

static void _dispatch_ctrl_rx(CANRxFrame *rx_msg)
{
    uint8_t dlc = rx_msg->DLC;
    if (dlc < 2) {
        log_info(LOG_PFX "Invalid control msg length: %i\r\n", dlc);
        return;
    }

    uint8_t ctrl_cmd = rx_msg->data8[0];
    switch(ctrl_cmd) {
        case CTRL_CMD_RESET_STN1110:
        {
            uint8_t protocol = rx_msg->data8[1];
            stn1110_reset(protocol);
            break;
        }
        default:
            log_info(LOG_PFX "Unknown control message command: %i\r\n", ctrl_cmd);
            break;
    }
}

static void _process_pid_request(CANRxFrame *rx_msg)
{

    if (!get_system_initialized())
        return;

    log_info(LOG_PFX "PID request\r\n");

    /* check if we're in the middle of a PID request,
     * and if so, did we time out? */
    if (get_pid_request_active()) {
        if (is_pid_request_timeout(get_obdii_request_timeout())) {
            log_info(LOG_PFX "Previous PID request timed out\r\n");
        }
        else{
            log_info(LOG_PFX "Ignoring, Previous PID request active\r\n");
            return;
        }
    }
    else{
        log_trace(LOG_PFX  "PID request not active\r\n");
    }

    uint8_t data_byte_count = rx_msg->data8[0];
    if (data_byte_count > MAX_PID_DATA_BYTES) {
        log_info(LOG_PFX "Invalid PID request; max data bytes %i exceeded %i\r\n", data_byte_count, MAX_PID_DATA_BYTES);
        return;
    }
    /* Write the PID request to the STN1110 */
    size_t i;
    for (i = 0; i < data_byte_count; i++) {
        chprintf((BaseSequentialStream *)&SD2, "%02X", rx_msg->data8[i + 1]);
    }
    chprintf((BaseSequentialStream *)&SD2, "\r");
    log_trace(LOG_PFX "Sent to STN1110\r\n");
    mark_stn1110_tx();
    set_pid_request_active(true);
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
            switch (rx_msg->EID){
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
            switch (rx_msg->SID){
                case OBDII_PID_REQUEST:
                    _process_pid_request(rx_msg);
                    break;
            }
        }
        break;
    }
}

void can_worker(void)
{
	  event_listener_t el;
	  CANRxFrame rx_msg;
	  chRegSetThreadName("CAN receiver");
	  chEvtRegister(&CAND1.rxfull_event, &el, 0);
	  while(!chThdShouldTerminateX()) {
	    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
	      continue;
	    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rx_msg, TIME_IMMEDIATE) == MSG_OK) {
	    	/* Process message.*/
	        log_CAN_rx_message(LOG_PFX, &rx_msg);
	        dispatch_can_rx(&rx_msg);
	    }
	  }
	  chEvtUnregister(&CAND1.rxfull_event, &el);
}
