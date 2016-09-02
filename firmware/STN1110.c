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

#define LOG_PFX "SYS_STN1110: "

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define STN1110_RUNTIME_BAUD_RATE 230400

/* Receive Buffer for the STN1110 */
char stn_rx_buf[1024];


static void _send_at(char *at_cmd)
{
    sdWrite(&SD2, (uint8_t*)at_cmd, strlen(at_cmd));
    chThdSleepMilliseconds(100);
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
    chThdSleepMilliseconds(10);
    palSetPad(GPIOB, GPIOB_RESET_STN1110);
    chThdSleepMilliseconds(1000);
    log_info(LOG_PFX "after hard reset\r\n");

    _send_at("AT E0\r");

    _send_at("AT SP 0\r");

    _send_at("AT DPN\r");

    _send_at("ST SBR " STR(STN1110_RUNTIME_BAUD_RATE) "\r");

    system_serial_init_SD2(STN1110_RUNTIME_BAUD_RATE);

    chThdSleepMilliseconds(1000);

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

static void _log_CAN_tx_message(CANTxFrame * can_frame)
{
    if (get_logging_level() < logging_level_trace)
        return;

    uint32_t CAN_id = can_frame->IDE == CAN_IDE_EXT ? can_frame->EID : can_frame->SID;
    log_trace(LOG_PFX "CAN Tx ID(%i): ", CAN_id);
    size_t i;
    for (i = 0; i < can_frame->DLC; i++)
    {
        log_trace("%02X ", can_frame->data8[i]);
    }
    log_trace("\r\n");
}
void _process_pid_response(char * buf)
{
    if (strstr(buf, "STOPPED") != 0) {
        log_info(LOG_PFX "stopped\r\n");
        chThdSleepMilliseconds(OBDII_PID_POLL_DELAY);
        set_pid_request_active(false);
    }
    else if (strstr(buf, "NO DATA") != 0) {
        log_info(LOG_PFX "no data\r\n");
        chThdSleepMilliseconds(OBDII_PID_POLL_DELAY);
        set_pid_request_active(false);
    }
    else if (_starts_with_hex(buf)) {
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
        /* Pause before transmitting the message
         * since the other system may immediately send the next PID request
         */
        chThdSleepMilliseconds(OBDII_PID_POLL_DELAY);
        canTransmit(&CAND1, CAN_ANY_MAILBOX, &can_pid_response, MS2ST(CAN_TRANSMIT_TIMEOUT));
        set_pid_request_active(false);
        log_info(LOG_PFX "CAN Tx\r\n");
        _log_CAN_tx_message(&can_pid_response);
    }
}

void stn1110_worker(void){
	stn1110_reset(0);
	while (true) {
		size_t bytes_read = serial_getline(&SD2, (uint8_t*)stn_rx_buf, sizeof(stn_rx_buf));
		if (bytes_read > 0) {
			//log_info("STN1110 rx (%i) %s ", bytes_read, stn_rx_buf);
			_process_pid_response(stn_rx_buf);
		}
	}
}

