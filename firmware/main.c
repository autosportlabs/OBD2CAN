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

/*
 * CAN receiver thread.
 */
static THD_WORKING_AREA(can_rx_wa, 512);
static THD_FUNCTION(can_rx, arg) {
	(void)arg;
	chRegSetThreadName("CAN_worker");
	can_worker();
}

/*
 * STN1110 receiver thread.
 */
static THD_WORKING_AREA(wa_STN1110_rx, 512);
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
	/* ChibiOS initialization */
	halInit();
	chSysInit();

	/* Application specific initialization */
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
