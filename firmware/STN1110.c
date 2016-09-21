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
#include "system_CAN.h"
#include "modp_numtoa.h"

#define _LOG_PFX "SYS_STN1110: "

#define RESET_DELAY 10
#define AT_COMMAND_DELAY 100
#define LONG_DELAY 1000

/* Receive Buffer for the STN1110 */
static char stn_rx_buf[1024];

/* Send an AT command */
static void _send_at(char *at_cmd)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    sdWrite(&SD2, (uint8_t*)"\r", 1);
    chThdSleepMilliseconds(AT_COMMAND_DELAY);
}

/* Send an AT command with a single numberic parameter value */
static void _send_at_param(char *at_cmd, int param)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    char param_str[16];
    modp_itoa10(param, param_str);
    sdWrite(&SD2, (uint8_t*)param_str, strlen(param_str));
    sdWrite(&SD2, (uint8_t*)"\r", 1);
    chThdSleepMilliseconds(AT_COMMAND_DELAY);
}

/* Send the STN1110 command to report the currently detected OBDII protocol */
static void _send_detect_protocol(void)
{
    /* Wait for STN1110 chip to be ready for command */
    chThdSleepMilliseconds(AT_COMMAND_DELAY);
    _send_at("AT DP");
}

/* Perform a hard reset of the STN1110 */
static void _hard_reset_stn1110(void)
{
    /*
     * set STN1110 NVM reset to disbled (normal running mode)
     * Use internall pullup resistor to disable NVM
     * TODO: this will be changed in hardware to just tie it to 3.3v
     * since we don't really need processor control of this pin
     * */
    palSetPadMode(GPIOA, GPIOB_RESET_NVM_STN1110, PAL_MODE_INPUT_PULLUP);

    /* Toggle hard reset Line */
    palSetPadMode(GPIOB, GPIOB_RESET_STN1110, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(RESET_DELAY);
    palSetPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(LONG_DELAY);
    log_info(_LOG_PFX "Hard reset STN1110\r\n");
}

/* Reset and configure the STN1110 */
void stn1110_reset(enum obdii_protocol protocol, enum obdii_adaptive_timing adaptive_timing, uint8_t obdii_timeout)
{
    set_system_initialized(false);
    log_info(_LOG_PFX "Reset STN1110 - protocol %i\r\n", protocol);

    _hard_reset_stn1110();

    system_serial_init_SD2(STN1110_INITIAL_BAUD_RATE);

    /* switch to the target baud rate */
    _send_at_param("ST SBR ", STN1110_RUNTIME_BAUD_RATE);
    system_serial_init_SD2(STN1110_RUNTIME_BAUD_RATE);

    /* set adaptive timing */
    _send_at_param("AT AT", adaptive_timing);

    /* set obdii protocol timeout */
    if (obdii_timeout > 0) {
        _send_at_param("AT ST ", obdii_timeout);
    }

    /* set protocol */
    _send_at_param("AT SP ", protocol);

    /* Disable echo */
    _send_at("AT E0");

    chThdSleepMilliseconds(LONG_DELAY);

    /* Reset our counters and flags */
    set_obdii_request_timeout(OBDII_INITIAL_TIMEOUT);
    set_pid_request_active(false);
    reset_pid_poll_delay();
    set_detected_protocol(obdii_protocol_auto);
    reset_nodata_error_count();
    reset_obdii_timeout_count();
    set_system_initialized(true);
}

static void _stn1110_reset_defaults(void)
{
    stn1110_reset(DEFAULT_OBDII_PROTOCOL, DEFAULT_OBDII_ADAPTIVE_TIMING, DEFAULT_OBDII_TIMEOUT);
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

/* check if the buffer starts with a 2 digit hex value */
static bool _starts_with_hex(char *buf)
{
    char first_char_str[4];
    strncpy(first_char_str, buf, 3);
    first_char_str[2] = '\0';
    uint8_t first_value;
    return _parse_byte(first_char_str, &first_value, 16);
}

/* Decode the OBDII protocol message and set the current protocol*/
static void _decode_protocol(const char * buf)
{
    if (strstr(buf, "SAE J1850 PWM") != 0) {
        set_detected_protocol(obdii_protocol_j1850_pwm);
        return;
    } else if (strstr(buf, "SAE J1850 VPW") != 0) {
        set_detected_protocol(obdii_protocol_j1850_vpw);
        return;
    } else if (strstr(buf, "ISO 9141-2") != 0) {
        set_detected_protocol(obdii_protocol_9141_2);
        return;
    } else if (strstr(buf, "ISO 14230-4") != 0) {
        set_detected_protocol(obdii_protocol_iso14230_4_kwp_5baud);
        return;
    }
}

/* Transform the hex string PID response from the STN1110 into the provided byte array */
static void _translate_pid_response(uint8_t * can_pid_response, char * buf)
{
    uint8_t pid_response[8];
    char *str_byte;
    char *save;
    str_byte = strtok_r(buf, " ", &save);
    size_t count = 0;
    /*
     * We can at most send 7 bytes in a message, since the first byte
     * is always the number of bytes following
     */
    while(str_byte != NULL && count < MAX_CAN_MESSAGE_SIZE - 1) {
        uint8_t byte;
        if (_parse_byte(str_byte, &byte, 16)) {
            pid_response[count++] = byte;
        }
        str_byte = strtok_r(NULL, " ", &save);
    }
    can_pid_response[0] = count;
    size_t i;
    for (i = 0; i < count; i++) {
        can_pid_response[i + 1] = pid_response[i];
    }
}

/* check if the STN1110 response contains a known error code */
enum STN1110_error _check_stn1110_error_response(const char *buf)
{
    /* So optimistic */
    enum STN1110_error stn1110_result = STN1110_ERROR_NONE;

    if (strstr(buf, "STOPPED") != 0) {
        log_info(_LOG_PFX "Stopped\r\n");
        stn1110_result = STN1110_ERROR_STOPPED;
        /*
         * When we get the STOPPED message it means we're
         * asking for data too fast. Stretch out the poll delay.
         */
        stretch_pid_poll_delay();
    }

    /* Handle a couple of known errors */
    else if (strstr(buf, "NO DATA") != 0) {
        log_info(_LOG_PFX "No data\r\n");
        stn1110_result = STN1110_ERROR_NO_DATA;
    } else if (strstr(buf, "ERROR") !=0 && strstr(buf, "BUS") != 0) {
    	/*
    	 * The STN1110 has a a couple of different error codes that
    	 * have the words ERROR and BUS in it. this check handles both
    	 * conditions.
    	 */
        log_info(_LOG_PFX "OBDII Bus error\r\n");
        stn1110_result = STN1110_ERROR_BUS_INIT;
    }

    return stn1110_result;
}

/* Broadcast the prepared CAN OBDII PID response */
void _send_can_pid_response(CANTxFrame *can_pid_response)
{
    /*
     * Pause before transmitting the message to limit update rate
     * since the other system may immediately send the next PID request
     */
    chThdSleepMilliseconds(get_pid_poll_delay());

    /* Now send the OBDII response */
    canTransmit(&CAND1, CAN_ANY_MAILBOX, can_pid_response, MS2ST(CAN_TRANSMIT_TIMEOUT));
    log_CAN_tx_message(_LOG_PFX, can_pid_response);

    /*
     * We've successfully processed at least one message;
     * set the timeout to the runtime timeout
     */
    set_obdii_request_timeout(OBDII_RUNTIME_TIMEOUT);
}

/* Request the detected protocol if the protocol is not yet known */
void _check_request_detected_protocol(void)
{
    /*
     * once we successfully get a PID response, ask
     * for the detected protocol if we still don't know what it is.
     * The response will be picked up the next time around in this
     * function.
     */
    if (get_detected_protocol() == obdii_protocol_auto) {
        _send_detect_protocol();
    }
}

/* Check for a response to the protocol detection request */
static bool _check_protocol_response(const char * buf)
{
    if (strstr(buf, "AUTO, ") != 0) {
        _decode_protocol(buf + 6);
        return true;
    }
    return false;
}

/* Process the response from the STN1110 */
void _process_stn1110_response(char * buf)
{
    /* filter invalid pointer */
    if (buf == NULL)
        return;

    /* Skip past AT command prompt, if present */
    if (buf[0] == '>')
        buf++;

    /* check if we got a protocol response */
    if (_check_protocol_response(buf))
        return;

    bool got_obd2_response = false;
    enum STN1110_error stn1110_result = _check_stn1110_error_response(buf);

    /* Did we collect an error? */
    if (stn1110_result != STN1110_ERROR_NONE) {
        increment_nodata_error_count();

        /* Save the current error code */
        set_stn1110_error(stn1110_result);

        /* Mark when we've received the message for our metrics */
        mark_stn1110_rx();

        got_obd2_response = true;
        chThdSleepMilliseconds(OBDII_PID_ERROR_DELAY);

    } else if (_starts_with_hex(buf)) {
        log_info(_LOG_PFX "PID reply\r\n");

        /*mark when we've received the message for our metrics */
        mark_stn1110_rx();

        /* request a protocol check, if needed */
        _check_request_detected_protocol();

        CANTxFrame can_pid_response;
        prepare_can_tx_message(&can_pid_response, CAN_IDE_STD, OBDII_PID_RESPONSE);

        /* translate the STN1110 PID response data into the equivalent CAN message */
        _translate_pid_response(can_pid_response.data8, buf);
        _send_can_pid_response(&can_pid_response);

        /* We got a response to the OBDII query */
        got_obd2_response = true;

        /* Note that we successfully got a PID response */
        set_stn1110_error(STN1110_ERROR_NONE);
        reset_nodata_error_count();
    }
    /*
     * If we got some sort of response - error or success
     * then mark ourselves as done.
     */
    if (got_obd2_response) {
        log_trace(_LOG_PFX "STN1110 latency: %ims\r\n", get_stn1110_latency());
        set_pid_request_active(false);
        reset_obdii_timeout_count();
    }
}

void send_stn1110_pid_request(uint8_t * data, size_t data_len)
{
    /*
     * check if we're in the middle of a PID request,
     * and if so, did we time out? */
    if (get_pid_request_active()) {
        if (is_pid_request_timeout(get_obdii_request_timeout())) {
            log_info(_LOG_PFX "Previous PID request timed out\r\n");
            increment_obdii_timeout_count();
            if (get_obdii_timeout_count() > MAX_OBDII_TIMEOUTS) {
                log_info(_LOG_PFX "Max timeouts, resetting system\r\n");
                reset_system();
            }
        } else {
            log_info(_LOG_PFX "Ignoring, Previous PID request active\r\n");
            return;
        }
    } else {
        log_trace(_LOG_PFX  "PID request not active\r\n");
    }

    /* Write the PID request to the STN1110 as a series of 2 digit hex values*/
    size_t i;
    for (i = 0; i < data_len; i++) {
        chprintf((BaseSequentialStream *)&SD2, "%02X", data[i]);
    }
    chprintf((BaseSequentialStream *)&SD2, "\r");
    log_trace(_LOG_PFX "Sent to STN1110\r\n");
    mark_stn1110_tx();
    set_pid_request_active(true);
}

void stn1110_worker(void)
{
    /*reset our chip */
    _stn1110_reset_defaults();

    while (true) {
        /* Wait for a line of data, then process it */
        size_t bytes_read = serial_getline(&SD2, (uint8_t*)stn_rx_buf, sizeof(stn_rx_buf));
        if (bytes_read > 0) {
            log_trace(_LOG_PFX "STN1110 raw Rx: %s\r\n", stn_rx_buf);
            if (get_system_initialized()) {
                _process_stn1110_response(stn_rx_buf);
            } else {
                log_trace(_LOG_PFX "Ignoring data from STN1110: system initializing\r\n");
            }
        }
    }
}

