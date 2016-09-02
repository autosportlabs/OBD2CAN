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

#include "ch.h"
#include "hal.h"
#include "settings.h"
#include "logging.h"
#include "STN1110.h"
#include "system.h"
#include "system_serial.h"
#include "system_CAN.h"

#define CAN_THREAD_STACK 512
#define STN1110_THREAD_STACK 512
#define MAIN_THREAD_SLEEP_MS 1000
#define WATCHDOG_TIMEOUT 1000

/*
 * CAN receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, CAN_THREAD_STACK);
static THD_FUNCTION(can_rx, arg) {
	(void)arg;
	chRegSetThreadName("CAN_worker");
	can_worker();
}

/*
 * STN1110 receiver thread.
 */
static THD_WORKING_AREA(wa_STN1110_rx, STN1110_THREAD_STACK);
static THD_FUNCTION(STN1110_rx, arg) {
	(void)arg;
	chRegSetThreadName("STN1110_worker");
	stn1110_worker();
}

/* Watchdog configuration and initialization
 */
static void _start_watchdog(void)
{
    const WDGConfig wdgcfg = {
            STM32_IWDG_PR_4,
          STM32_IWDG_RL(WATCHDOG_TIMEOUT)
        };
    wdgStart(&WDGD1, &wdgcfg);
}

int main(void) {
	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/
	/* ChibiOS initialization */
	halInit();
	chSysInit();
	//_start_watchdog();

	/* Application specific initialization */
	system_can_init();
	system_serial_init();

	/*
	* Creates the processing threads.
	*/
	chThdCreateStatic(wa_STN1110_rx, sizeof(wa_STN1110_rx), NORMALPRIO, STN1110_rx, NULL);
	chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO, can_rx, NULL);

	while (true) {
		chThdSleepMilliseconds(MAIN_THREAD_SLEEP_MS);
        //wdgReset(&WDGD1);
	}
	return 0;
}
