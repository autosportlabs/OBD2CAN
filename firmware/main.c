/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include <string.h>
#include "chprintf.h"
#include "pal_lld.h"
/**
 * STN1110 data Receive thread
 */


/*
 * 500KBaud, automatic wakeup, automatic recover
 * from abort mode.

CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
 */



//CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP
static const CANConfig cancfg = {
        CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP  ,
  CAN_BTR_SJW(1) | CAN_BTR_TS2(3) |
  CAN_BTR_TS1(13) | CAN_BTR_BRP(6)
};
char stn_rx_buf[1024] = "booo\r\n";

static void debug_write(char *msg)
{
    sdWrite(&SD1, (uint8_t*)"stuff: ", 7);
    sdWrite(&SD1, (uint8_t*)msg, strlen(msg));
    sdPut(&SD1, '\r');
    sdPut(&SD1, '\n');
}

static void reset_stn1110(void)
{
    debug_write("resetting");

    /* set STN1110 NVM reset to disbled (normal running mode)
     * Use internall pullup resistor to disable NVM
     * */
    palSetPadMode(GPIOA, GPIOB_RESET_NVM_STN1110, PAL_MODE_INPUT_PULLUP);

    /* Toggle Reset Line */
    palSetPadMode(GPIOB, GPIOB_RESET_STN1110, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(10);
    palSetPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(1000);
    debug_write("after reset");
}

static void send_at(char *at_cmd)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    chThdSleepMilliseconds(1000);
}

static THD_WORKING_AREA(wa_STN1110_rx, 128);
static THD_FUNCTION(STN1110_rx, arg) {
  (void)arg;
  chRegSetThreadName("STN1110_RX");

  reset_stn1110();
  send_at("AT E0\r");
  send_at("AT SP 0\r");

  while (true) {
      /* Reset the STN1110 */
      send_at("010C\r");
      debug_write("Waiting for AT response");
      int bytes_read = sdReadTimeout(&SD2,(uint8_t*)stn_rx_buf,sizeof(stn_rx_buf), 50000);
      stn_rx_buf[bytes_read] = '\0';
      chprintf(&SD1, "Bytes read %i\r\n", bytes_read);
      debug_write(stn_rx_buf);

  }
}


/*
 * Receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, p) {
  event_listener_t el;
  CANRxFrame rxmsg;

  chprintf(&SD1, "freq %i\r\n", STM32_HCLK);

  debug_write("CAN Rx starting");
  (void)p;
  chRegSetThreadName("receiver");
  chEvtRegister(&CAND1.rxfull_event, &el, 0);
  while(!chThdShouldTerminateX()) {
    if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
      continue;
    while (canReceive(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_IMMEDIATE) == MSG_OK) {
      /* Process message.*/
        debug_write("CAN Rx");
    }
  }
  chEvtUnregister(&CAND1.rxfull_event, &el);
}


/*
 * Transmitter thread.
 */
static THD_WORKING_AREA(can_tx_wa, 256);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (!chThdShouldTerminateX()) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, MS2ST(100));
    chThdSleepMilliseconds(1000);
    debug_write("CAN Tx");
  }
}

/**
 * CAN data receive thread
static THD_WORKING_AREA(wa_CAN_rx, 128);
static THD_FUNCTION(CAN_rx2, arg) {

    (void)arg;
    chThdSleepMilliseconds(2000);
    chRegSetThreadName("CAN_RX");
   // send_at("AT E0\r");
    //send_at("AT SP 0\r");

    while (true) {
        debug_write("sending AT");
        send_at("AT\r\n");
        chThdSleepMilliseconds(1000);
    }
}
 */

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();


  /* CAN RX.       */
  palSetPadMode(GPIOA, 11, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));
  /* CAN TX.       */
  palSetPadMode(GPIOA, 12, PAL_STM32_MODE_ALTERNATE | PAL_STM32_ALTERNATE(4));

  /*
   * Activates the CAN driver 1.
   */
  canStart(&CAND1, &cancfg);


  /* Initialize connection to STN1110 on SD2
   */
  static SerialConfig stn_uart_cfg;
  stn_uart_cfg.speed=9600;

  /* USART2 TX.       */
  palSetPadMode(GPIOA, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(1));

  /* USART2 RX.       */
  palSetPadMode(GPIOA, 3, PAL_STM32_MODE_ALTERNATE | PAL_STM32_PUPDR_PULLUP | PAL_STM32_ALTERNATE(1));

  sdStart(&SD2, NULL);

  /*
   * Activates the serial driver 1 (debug port) using the driver default configuration.
   * PA9 and PA10 are routed to USART1.
   */
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */
  sdStart(&SD1, NULL);

  /*
   * Creates the processing threads.
   */
  chThdCreateStatic(wa_STN1110_rx, sizeof(wa_STN1110_rx), NORMALPRIO, STN1110_rx, NULL);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7, can_tx, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state, when the button is
   * pressed the test procedure is launched with output on the serial
   * driver 1.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
