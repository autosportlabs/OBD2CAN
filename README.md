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

