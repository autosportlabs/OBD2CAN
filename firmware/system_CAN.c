/*
 * CAN.c
 *
 *  Created on: Aug 31, 2016
 *      Author: brent
 */
#include "system_CAN.h"
#include "logging.h"
#include "system_serial.h"
#include "settings.h"
#include "system.h"
#include "STN1110.h"

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
        debug_write("Invalid control msg length: %i\r\n", dlc);
        return;
    }

    uint8_t ctrl_cmd = rx_msg->data8[0];
    switch(ctrl_cmd) {
        case CTRL_CMD_RESET:
        {
            uint8_t protocol = rx_msg->data8[1];
            stn1110_reset(protocol);
            break;
        }
        default:
            debug_write("Unknown control message command: %i", ctrl_cmd);
            break;
    }
}

static void _process_pid_request(CANRxFrame *rx_msg)
{
    if (!get_system_initialized())
        return;

    if (get_pid_request_active())
        return;

    uint8_t pid = rx_msg->data8[2];
    //debug_write("received PID request %i", pid);
    chprintf((BaseSequentialStream *)&SD2, "01%02X\r", pid);
    set_pid_request_active(true);
    //chprintf((BaseSequentialStream *)&SD2, "010C\r");
    //send_at("010C\r");
    //sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd))
}

void dispatch_can_rx(CANRxFrame *rx_msg)
/*
 * Dispatch an incoming CAN message
 */
{       //send_at("010C\r");

    /* we are only handling extended IDs */

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
	  CANRxFrame rxmsg;

	  debug_write("freq %i\r\n", STM32_HCLK);

	  debug_write("CAN Rx starting");
	  chRegSetThreadName("receiver");
	  chEvtRegister(&CAND1.rxfull_event, &el, 0);
	  while(!chThdShouldTerminateX()) {
	    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
	      continue;
	    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
	      /* Process message.*/
	        debug_write("CAN Rx");
	        dispatch_can_rx(&rxmsg);
	    }
	  }
	  chEvtUnregister(&CAND1.rxfull_event, &el);
}
