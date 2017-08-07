OBD2CAN
=======

OBD2CAN allows cars with pre-CAN OBDII enabled ECUs (1996-2007) to emulate a CAN-OBDII interface.

Target
======

OBD2CAN was originally designed as an enhancement to the RaceCapture and RaceCapture/Pro line of hardware, but could theoretically be used with any device that can only communicate with CAN-OBDII enabled ECUs.

Principal of Operation
======================

The operation of OBD2CAN is straightforward:

CAN-OBDII requests are received by OBD2CAN and are forwarded on to the legacy-OBDII interface, using
a STN1110 processor and interface circuitry. 

Legacy-OBDII PID responses are received from the STN1110, and transformed back into the equivalent CAN-OBDII response.

Firmware
========
The firmware is targeted for the STM32F072, and uses ChibiOS as a RTOS. 

Compiling
=========
Compiling requires arm-none-eabi gcc version 4.7.4 20130913

From the firmware directory issue:

> make clean

> make

Flashing
========
* ST bootloader

The firmware image can be flashed using the ST bootloader using the USART1 header after activating BOOT mode. On the OBD2CAN hardware the BOOT jumper pads is adjacent to the USART1 header. The USART header can be connected to a common FTDI 3.3v USB-USART cable (TTL-232R-3V3).

* JTAG SWD

An openocd script is provided to flash the STM32 via SWD, using a ST-LINK/V2

Operation
=========

CAN PID requests are received per OBDII SAE specifications and are translated 1:1 to STN1110 PID requests for the target ECU. 
* 500KBPS
* 11 bit

Protocol is set for automatic detection. 


Diagnostic and command/control CAN messages
===========================================

Control Message
===============
The control message allows some control over the OBD2CAN module's settings

CAN ID: 62344

* Byte 0: Sub command.
 * always 0x01

* Byte 1: Debug level on UART header
 * 0: no logging
 * 1: info level logging
 * 2: trace level logging

* Byte 2: Reset STN1110
 * 0: do not reset
 * 1: perform a hard reset

* Byte 3: Set OBDII protocol
Affects AT SP command, see ELM327 manual. If different than the current setting, triggers a reset of the STN1110
 * 0: auto detect (power up default)
 * 1: J1850 PWM
 * 2: J1850 VPW
 * 3: ISO 9141-2
 * 4: ISO 14230-4

Diagnostic
==========
The OBD2CAN will broadcast a diagnostic message at 1Hz with the following information:

CAN ID: 62345

* Byte 0: Detected protocol
 * 0: auto detect (still detecting)
 * 1: J1850 PWM
 * 2: J1850 VPW
 * 3: ISO 9141-2
 * 4: ISO 14230-4

* Byte 1: Last error code
 * 0: STN1110_ERROR_NONE,
 * 1: STN1110_ERROR_STOPPED,
 * 2: STN1110_ERROR_NO_DATA,
 * 3: STN1110_ERROR_BUS_INIT

* Byte 2-3: STN1110 request/reply latency, in ms. 
 * Little Endian

* Byte 4: reserved

* Byte 5: firmware major version

* Byte 6: firmware minor version

* Byte 7: firmware patch version







