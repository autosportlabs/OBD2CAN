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
#include "pal_lld.h"
#include "STN1110.h"
#include "system_serial.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>
#include "settings.h"
#include "system.h"
#include "modp_numtoa.h"

#define LOG_PFX "SYS_STN1110: "

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define STN1110_RUNTIME_BAUD_RATE 230400
#define RESET_DELAY 10
#define AT_COMMAND_DELAY 100
#define LONG_DELAY 1000

systime_t stn1110_last_message_at = 0;

/* Receive Buffer for the STN1110 */
char stn_rx_buf[1024];

static void _send_at(char *at_cmd)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    sdWrite(&SD2, (uint8_t*)"\r", 1);
    chThdSleepMilliseconds(AT_COMMAND_DELAY);
}

static void _send_at_param(char *at_cmd, int param)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    char param_str[20];
    modp_itoa10(param, param_str);
    sdWrite(&SD2, (uint8_t*)param_str, strlen(param_str));
    sdWrite(&SD2, (uint8_t*)"\r", 1);
    chThdSleepMilliseconds(AT_COMMAND_DELAY);
}

void stn1110_reset(uint8_t protocol)
{
	set_system_initialized(false);
    log_info(LOG_PFX "Reset STN1110 - protocol %i\r\n", protocol);

    /* set STN1110 NVM reset to disbled (normal running mode)
     * Use internall pullup resistor to disable NVM
     * */
    palSetPadMode(GPIOA, GPIOB_RESET_NVM_STN1110, PAL_MODE_INPUT_PULLUP);

    /* Toggle hard reset Line */
    palSetPadMode(GPIOB, GPIOB_RESET_STN1110, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(RESET_DELAY);
    palSetPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(LONG_DELAY);
    log_info(LOG_PFX "after hard reset\r\n");

    /* switch to the target baud rate */
    _send_at_param("ST SBR ", STN1110_RUNTIME_BAUD_RATE);
    system_serial_init_SD2(STN1110_RUNTIME_BAUD_RATE);

    /* set protocol */
    _send_at_param("AT SP ", protocol);

    /* Disable echo */
    _send_at("AT E0");

    chThdSleepMilliseconds(LONG_DELAY);

    /* set our initial OBDII timeout
     * needed when the interface first detects the
     * protocol
     */
    set_obdii_request_timeout(OBDII_INITIAL_TIMEOUT);
    set_system_initialized(true);
}

static bool _parse_byte(const char *str, uint8_t *val, int base)
{
    char *temp;
    bool rc = true;
    *val = strtol(str, &temp, base);

    if (temp == str || *temp != '\0')
        rc = false;
    return rc;
}

/*
 * check if the buffer starts with a 2 digit hex value
 */
static bool _starts_with_hex(char *buf)
{
    char first_char_str[4];
    strncpy(first_char_str, buf, 3);
    first_char_str[2] = '\0';
    uint8_t first_value;
    return _parse_byte(first_char_str, &first_value, 16);
}


void _process_pid_response(char * buf)
{
    bool got_obd2_response = false;
    if (strstr(buf, "STOPPED") != 0) {
        log_info(LOG_PFX "stopped\r\n");
        got_obd2_response = true;
        mark_stn1110_rx();
        set_stn1110_error(STN1110_ERROR_STOPPED);
    }
    else if (strstr(buf, "NO DATA") != 0) {
        log_info(LOG_PFX "no data\r\n");
        got_obd2_response = true;
        mark_stn1110_rx();
        set_stn1110_error(STN1110_ERROR_NO_DATA);
    }
    else if (strstr(buf, "BUS INIT: ...ERROR") !=0){
        log_info(LOG_PFX "Bus init error\r\n");
        got_obd2_response = true;
        mark_stn1110_rx();
        set_stn1110_error(STN1110_ERROR_BUS_INIT);
    }
    else if (_starts_with_hex(buf)) {
    	/* Translate the STN1110 PID response to
    	 * the equivalent CAN message */
        log_info(LOG_PFX "PID reply\r\n");
        CANTxFrame can_pid_response;
        can_pid_response.IDE = CAN_IDE_STD;
        can_pid_response.SID = OBDII_PID_RESPONSE;
        can_pid_response.RTR = CAN_RTR_DATA;
        can_pid_response.DLC = 8;
        can_pid_response.data8[0] = 0x55;
        can_pid_response.data8[1] = 0x55;
        can_pid_response.data8[2] = 0x55;
        can_pid_response.data8[3] = 0x55;
        can_pid_response.data8[4] = 0x55;
        can_pid_response.data8[5] = 0x55;
        can_pid_response.data8[6] = 0x55;
        can_pid_response.data8[7] = 0x55;

        uint8_t pid_response[8];
        char *str_byte;
        char *save;
        str_byte = strtok_r(buf, " ", &save);
        size_t count = 0;
        while(str_byte != NULL && count < MAX_CAN_MESSAGE_SIZE)
        {
            uint8_t byte;
            if (_parse_byte(str_byte, &byte, 16)){
                pid_response[count++] = byte;
            }
            str_byte = strtok_r(NULL, " ", &save);
        }
        can_pid_response.data8[0] = count;
        size_t i;
        for (i = 0; i < count; i++) {
            can_pid_response.data8[i + 1] = pid_response[i];
        }
        mark_stn1110_rx();
        log_trace(LOG_PFX "STN1110 latency: %ims\r\n", get_stn1110_latency());

        /* Pause before transmitting the message to limit update rate
         * since the other system may immediately send the next PID request
         */
        chThdSleepMilliseconds(OBDII_PID_POLL_DELAY);

        /* Now send the OBDII response */
        canTransmit(&CAND1, CAN_ANY_MAILBOX, &can_pid_response, MS2ST(CAN_TRANSMIT_TIMEOUT));
        log_CAN_tx_message(LOG_PFX, &can_pid_response);

        got_obd2_response = true;

        /* We've successfully received at least one message;
         *set the timeout to the runtime timeout
         */
        set_obdii_request_timeout(OBDII_RUNTIME_TIMEOUT);
    }

    if (got_obd2_response) {
        set_pid_request_active(false);
        log_trace(LOG_PFX "STN1110 response latency: %ims\r\n", get_stn1110_latency());
    }
}

void stn1110_worker(void){
	stn1110_reset(3);
	while (true) {
		size_t bytes_read = serial_getline(&SD2, (uint8_t*)stn_rx_buf, sizeof(stn_rx_buf));
		if (bytes_read > 0) {
			log_trace(LOG_PFX "STN1110 raw Rx: %s\r\n", stn_rx_buf);
            _process_pid_response(stn_rx_buf);
		}
	}
}

