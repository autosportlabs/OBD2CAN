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
#include "settings.h"
#include "logging.h"
#include "STN1110.h"
#include "system.h"
#include "system_serial.h"
#include "system_CAN.h"

/*
 * CAN receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, arg) {
	(void)arg;
	chRegSetThreadName("CAN_worker");
	can_worker();
}

/*
 * STN1110 receiver thread.
 */
static THD_WORKING_AREA(wa_STN1110_rx, 256);
static THD_FUNCTION(STN1110_rx, arg) {
	(void)arg;
	chRegSetThreadName("STN1110_worker");
	stn1110_worker();
}

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
  system_can_init();
  system_serial_init();

  /*
   * Creates the processing threads.
   */
  chThdCreateStatic(wa_STN1110_rx, sizeof(wa_STN1110_rx), NORMALPRIO, STN1110_rx, NULL);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7, can_rx, NULL);

  /*
   * Main thread sleeps.
   */
  while (true) {
    chThdSleepMilliseconds(1000);
  }
}
