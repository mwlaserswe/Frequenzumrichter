/**
 * Lauflicht mit den grünen LED's
 */





/**
 * \mainpage
 * \section intro Introduction
 * This example demonstrates how to use the common IOPORT service for port
 * control.
 *
 * \section files Main Files
 *  - ioport.h common gpio definitions
 *  - ioport_example1.c example application
 *  - conf_example.h example definitions
 *
 * \section ioport_service_section services/ioport/ioport.h
 * The common IOPORT service is described in
 * \ref ioport_group section.
 *
 * \section device_info Device Info
 * All Atmel SAM, MEGA, XMEGA and UC3 devices can be used. This example has been
 * tested with the following setup:
 *  - Xplain evaluation kit
 *  - EVK1100 evaluation kit
 *  - SAM3X evaluation kit
 *  - SAM4L evaluation kit
 *  - SAM4L Xplained Pro kit
 *  - SAM4L8 Xplained Pro kit
 *  - SAM4E evaluation kit
 *  - SAM4N Xplained Pro kit
 *  - STK600 evaluation Kit (Tested with STK600-ATMEGA128RFA1)
 *  - ATmega256RFR2 Xplained Pro kit
 *  - SAM4C evaluation kit
 *  - SAM4CP16BMB evaluation board (no button version)
 *  - SAMG53 Xplained Pro kit
 *  - SAMG55 Xplained Pro kit
 *  - SAM4CMP16C demo board
 *  - SAM4CMS16C demo board
 *  - SAMV71 Xplained Ultra kit
 *  - SAME70 Xplained Pro kit
 *
 * \section example_description Description of the example
 * This example flashes a group of LEDs present on a port with flash speed of
 * 1Hz.
 * For STK600 with STK600-ATMEGA128RFA1, need to connect a 10 Pin cable from
 * PORTB to Led header
 * Additionally a group of inputs can inspected from a debug session.
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for SAM and AVR.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit <a href="http://www.atmel.com/">Atmel</a>.\n
 * Support and FAQ: https://www.microchip.com/support/
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "conf_example.h"

int main(void)
{
	/* Use static volatile to make it available in debug watch */
	static volatile ioport_port_mask_t port_val;

	sysclk_init();
	board_init();
	ioport_init();

	delay_init(sysclk_get_cpu_hz());

	/* Set output direction on the given LED IOPORTs */
	ioport_set_port_dir(EXAMPLE_LED_PORT, EXAMPLE_LED_MASK, IOPORT_DIR_OUTPUT);

	/* Set LED IOPORTs high */
	ioport_set_port_level(EXAMPLE_LED_PORT, EXAMPLE_LED_MASK, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(1000);
	
	while (true) {

		// Lauflicht mit den grünen LED's
		// Doku: https://www.mikrocontroller.net/articles/AVR32_Tutorial
		AVR32_GPIO.port[1].ovr = 0x70000000;
		delay_ms(500);
		AVR32_GPIO.port[1].ovr = 0x68000000;
		delay_ms(500);
		AVR32_GPIO.port[1].ovr = 0x58000000;
		delay_ms(500);
		AVR32_GPIO.port[1].ovr = 0x38000000;
		delay_ms(500);
	}
}
