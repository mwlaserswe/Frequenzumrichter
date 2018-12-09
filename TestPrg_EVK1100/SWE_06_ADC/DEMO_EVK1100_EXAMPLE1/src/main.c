/*This file has been prepared for Doxygen automatic documentation generation.*/
#include "board.h"
#include "print_funcs.h"
#include "gpio.h"
#include "pm.h"
#include "adc.h"



#if BOARD == EVK1100
// Connection of the temperature sensor
#  define EXAMPLE_ADC_TEMPERATURE_CHANNEL     0
#  define EXAMPLE_ADC_TEMPERATURE_PIN         AVR32_ADC_AD_0_PIN
#  define EXAMPLE_ADC_TEMPERATURE_FUNCTION    AVR32_ADC_AD_0_FUNCTION
// Connection of the light sensor
#  define EXAMPLE_ADC_LIGHT_CHANNEL           2
#  define EXAMPLE_ADC_LIGHT_PIN               AVR32_ADC_AD_2_PIN
#  define EXAMPLE_ADC_LIGHT_FUNCTION          AVR32_ADC_AD_2_FUNCTION
// Connection of the potentiometer
#  define EXAMPLE_ADC_POTENTIOMETER_CHANNEL   1
#  define EXAMPLE_ADC_POTENTIOMETER_PIN       AVR32_ADC_AD_1_PIN
#  define EXAMPLE_ADC_POTENTIOMETER_FUNCTION  AVR32_ADC_AD_1_FUNCTION
// Note: Corresponding defines are defined in /BOARDS/EVK1100/evk1100.h.
// These are here for educational purposes only.
#elif BOARD == EVK1101
// Connection of the temperature sensor
#  define EXAMPLE_ADC_TEMPERATURE_CHANNEL     7
#  define EXAMPLE_ADC_TEMPERATURE_PIN         AVR32_ADC_AD_7_PIN
#  define EXAMPLE_ADC_TEMPERATURE_FUNCTION    AVR32_ADC_AD_7_FUNCTION
// Connection of the light sensor
#  define EXAMPLE_ADC_LIGHT_CHANNEL           6
#  define EXAMPLE_ADC_LIGHT_PIN               AVR32_ADC_AD_6_PIN
#  define EXAMPLE_ADC_LIGHT_FUNCTION          AVR32_ADC_AD_6_FUNCTION
// Note: Corresponding defines are defined in /BOARDS/EVK1101/evk1101.h.
// These are here for educational purposes only.
#elif BOARD == EVK1104
// Connection of the light sensor
#  define EXAMPLE_ADC_LIGHT_CHANNEL           0
#  define EXAMPLE_ADC_LIGHT_PIN               AVR32_ADC_AD_0_PIN
#  define EXAMPLE_ADC_LIGHT_FUNCTION          AVR32_ADC_AD_0_FUNCTION
// Note: Corresponding defines are defined in /BOARDS/EVK1104/evk1104.h.
// These are here for educational purposes only.
#endif

volatile int true_var = 1;

int main( void )
{
	// GPIO pin/adc-function map.
	static const gpio_map_t ADC_GPIO_MAP =
	{
		#if BOARD == EVK1100 || BOARD == EVK1101
		{EXAMPLE_ADC_TEMPERATURE_PIN, EXAMPLE_ADC_TEMPERATURE_FUNCTION},
		#endif
		{EXAMPLE_ADC_LIGHT_PIN, EXAMPLE_ADC_LIGHT_FUNCTION},
		#if BOARD == EVK1100
		{EXAMPLE_ADC_POTENTIOMETER_PIN, EXAMPLE_ADC_POTENTIOMETER_FUNCTION}
		#endif
	};

	volatile avr32_adc_t *adc = &AVR32_ADC; // ADC IP registers address

	#if BOARD == EVK1100 || BOARD == EVK1101
	volatile signed short adc_value_temp = -1;
	#endif
	volatile signed short adc_value_light = -1;
	#if BOARD == EVK1100
	volatile signed short adc_value_pot = -1;
	#endif

	// Assign the on-board sensors to their ADC channel.
	#if BOARD == EVK1100 || BOARD == EVK1101
	unsigned short adc_channel_temp = EXAMPLE_ADC_TEMPERATURE_CHANNEL;
	#endif
	unsigned short adc_channel_light = EXAMPLE_ADC_LIGHT_CHANNEL;
	#if BOARD == EVK1100
	unsigned short adc_channel_pot = EXAMPLE_ADC_POTENTIOMETER_CHANNEL;
	#endif

	int i;


	// switch to oscillator 0
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	// init debug serial line
	init_dbg_rs232(FOSC0);

	// Assign and enable GPIO pins to the ADC function.
	gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) / sizeof(ADC_GPIO_MAP[0]));

	// configure ADC
	// Lower the ADC clock to match the ADC characteristics (because we configured
	// the CPU clock to 12MHz, and the ADC clock characteristics are usually lower;
	// cf. the ADC Characteristic section in the datasheet).
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(adc);

	// Enable the ADC channels.
	#if BOARD == EVK1100 || BOARD == EVK1101
	adc_enable(adc,adc_channel_temp);
	#endif
	adc_enable(adc,adc_channel_light);
	#if BOARD == EVK1100
	adc_enable(adc,adc_channel_pot);
	#endif


	// do an infinite loop
	while (true_var)    // use a volatile true variable to avoid warning on unreachable code
	{
		// slow down operations
		for ( i=0 ; i < 1000000 ; i++);

		// display a header to user
		print_dbg("\x1B[2J\x1B[H\r\nADC Example\r\n");

		// launch conversion on all enabled channels
		adc_start(adc);
		
		#if BOARD == EVK1100 || BOARD == EVK1101
		// get value for the temperature adc channel
		adc_value_temp = adc_get_value(adc, adc_channel_temp);
		// display value to user
		print_dbg("HEX Value for Channel temperature : 0x");
		print_dbg_hex(adc_value_temp);
		print_dbg("\r\n");
		#endif

		// get value for the light adc channel
		adc_value_light = adc_get_value(adc, adc_channel_light);
		// display value to user
		print_dbg("HEX Value for Channel light : 0x");
		print_dbg_hex(adc_value_light);
		print_dbg("\r\n");

		#if BOARD == EVK1100
		// get value for the potentiometer adc channel
		adc_value_pot = adc_get_value(adc, adc_channel_pot);
		// display value to user
		print_dbg("HEX Value for Channel pot : 0x");
		print_dbg_hex(adc_value_pot);
		print_dbg("\r\n");
		#endif
	}
	// Disable the ADC channels.
	#if BOARD == EVK1100 || BOARD == EVK1101
	adc_disable(adc,adc_channel_temp);
	#endif
	adc_disable(adc,adc_channel_light);
	#if BOARD == EVK1100
	adc_disable(adc,adc_channel_pot);
	#endif
	return 0;
}

