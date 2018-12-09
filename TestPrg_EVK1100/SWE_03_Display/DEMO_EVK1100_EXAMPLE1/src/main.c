/*This file has been prepared for Doxygen automatic documentation generation.*/
#include "board.h"
#include "compiler.h"
#include "dip204.h"
#include "intc.h"
#include "gpio.h"
#include "pm.h"
#include "delay.h"
#include "spi.h"
#include <avr32/io.h>


#define GPIO_CHARSET            GPIO_PUSH_BUTTON_0

#define GPIO_BACKLIGHT_MINUS    GPIO_PUSH_BUTTON_1

#define GPIO_BACKLIGHT_PLUS     GPIO_PUSH_BUTTON_2

unsigned short display;

unsigned short current_char = 0;


#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void dip204_example_PB_int_handler(void)
{
	unsigned short i;

	/* display all available chars */
	if (gpio_get_pin_interrupt_flag(GPIO_CHARSET))
	{
		/* go to first column of 4th line */
		dip204_set_cursor_position(1,4);
		/* display 20 chars of charmap */
		for (i = current_char; i < current_char + 0x10; i++)
		{
			dip204_write_data(i);
		}
		dip204_write_string("    ");
		/* mark position in charmap */
		current_char = i;
		/* reset marker */
		if (current_char >= 0xFF)
		{
			current_char = 0x10;
		}
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_CHARSET);
	}
	/* increase backlight power */
	if (gpio_get_pin_interrupt_flag(GPIO_BACKLIGHT_PLUS))
	{
		dip204_set_backlight(backlight_power_increase);
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_BACKLIGHT_PLUS);
	}
	/* decrease backlight power */
	if (gpio_get_pin_interrupt_flag(GPIO_BACKLIGHT_MINUS))
	{
		dip204_set_backlight(backlight_power_decrease);
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_BACKLIGHT_MINUS);
	}
}


#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
__interrupt
#endif
static void dip204_example_Joy_int_handler(void)
{
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_UP))
	{
		dip204_set_cursor_position(19,1);
		dip204_write_data(0xDE);
		display = 1;
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_UP);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_DOWN))
	{
		dip204_set_cursor_position(19,3);
		dip204_write_data(0xE0);
		display = 1;
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_LEFT))
	{
		dip204_set_cursor_position(18,2);
		dip204_write_data(0xE1);
		display = 1;
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT))
	{
		dip204_set_cursor_position(20,2);
		dip204_write_data(0xDF);
		display = 1;
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(GPIO_JOYSTICK_PUSH))
	{
		dip204_set_cursor_position(19,2);
		dip204_write_data(0xBB);
		dip204_set_cursor_position(1,4);
		dip204_write_string("  AT32UC3A Series   ");
		display = 1;
		/* allow new interrupt : clear the IFR flag */
		gpio_clear_pin_interrupt_flag(GPIO_JOYSTICK_PUSH);
	}
}


void dip204_example_configure_push_buttons_IT(void)
{
	gpio_enable_pin_interrupt(GPIO_CHARSET , GPIO_RISING_EDGE);

	gpio_enable_pin_interrupt(GPIO_BACKLIGHT_PLUS , GPIO_RISING_EDGE);

	gpio_enable_pin_interrupt(GPIO_BACKLIGHT_MINUS , GPIO_RISING_EDGE);

	/* Disable all interrupts */
	Disable_global_interrupt();
	/* register PB0 handler on level 1 */
	INTC_register_interrupt( &dip204_example_PB_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_BACKLIGHT_PLUS/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_PB_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_BACKLIGHT_MINUS/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_PB_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_CHARSET/8), AVR32_INTC_INT1);
	/* Enable all interrupts */
	Enable_global_interrupt();
}


void dip204_example_configure_joystick_IT(void)
{
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_UP , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_DOWN , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_RIGHT , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_PUSH , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(GPIO_JOYSTICK_LEFT , GPIO_FALLING_EDGE);

	/* Disable all interrupts */
	Disable_global_interrupt();
	/* register PB0 handler on level 1 */
	INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_UP/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_DOWN/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_RIGHT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_LEFT/8), AVR32_INTC_INT1);
	INTC_register_interrupt( &dip204_example_Joy_int_handler, AVR32_GPIO_IRQ_0 + (GPIO_JOYSTICK_PUSH/8), AVR32_INTC_INT1);
	/* Enable all interrupts */
	Enable_global_interrupt();
}

int main(void)
{
	static const gpio_map_t DIP204_SPI_GPIO_MAP =
	{
		{DIP204_SPI_SCK_PIN,  DIP204_SPI_SCK_FUNCTION },  // SPI Clock.
		{DIP204_SPI_MISO_PIN, DIP204_SPI_MISO_FUNCTION},  // MISO.
		{DIP204_SPI_MOSI_PIN, DIP204_SPI_MOSI_FUNCTION},  // MOSI.
		{DIP204_SPI_NPCS_PIN, DIP204_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};

	// Switch the CPU main clock to oscillator 0
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	// Disable all interrupts.
	Disable_global_interrupt();

	// init the interrupts
	INTC_init_interrupts();

	// Enable all interrupts.
	Enable_global_interrupt();

	// add the spi options driver structure for the LCD DIP204
	spi_options_t spiOptions =
	{
		.reg          = DIP204_SPI_NPCS,
		.baudrate     = 1000000,
		.bits         = 8,
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI
	gpio_enable_module(DIP204_SPI_GPIO_MAP,
	sizeof(DIP204_SPI_GPIO_MAP) / sizeof(DIP204_SPI_GPIO_MAP[0]));

	// Initialize as master
	spi_initMaster(DIP204_SPI, &spiOptions);

	// Set selection mode: variable_ps, pcs_decode, delay
	spi_selectionMode(DIP204_SPI, 0, 0, 0);

	// Enable SPI
	spi_enable(DIP204_SPI);

	// setup chip registers
	spi_setupChipReg(DIP204_SPI, &spiOptions, FOSC0);

	// configure local push buttons
	dip204_example_configure_push_buttons_IT();

	// configure local joystick
	dip204_example_configure_joystick_IT();

	// initialize delay driver
	delay_init( FOSC0 );

	// initialize LCD
	dip204_init(backlight_PWM, true);

	// reset marker
	current_char = 0x10;


	/* do a loop */
	while (1)
	{
		dip204_clear_display();
		delay_ms(200);
		dip204_set_cursor_position(8,1);
		dip204_write_string("ATMEL");
		delay_ms(200);
		dip204_set_cursor_position(7,2);
		dip204_write_string("EVK1100");
		delay_ms(200);
		dip204_set_cursor_position(6,3);
		dip204_write_string("AVR32 UC3");
		delay_ms(200);
		dip204_set_cursor_position(3,4);
		dip204_write_string("AT32UC3A Series");
		dip204_hide_cursor();
		delay_ms(200);
	}
}