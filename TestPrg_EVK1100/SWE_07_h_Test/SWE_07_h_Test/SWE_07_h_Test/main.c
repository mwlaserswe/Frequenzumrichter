/*
 * SWE_07_h_Test.c
 *
 * Created: 28.11.2018 13:56:57
 * Author : j1
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

volatile uint8_t BitMuster;

int main(void)
{
    /* Replace with your application code */
	DDRB = 0xFF;
	
    while (1) 
    {
		BitMuster = 0xAA;
		PORTB = BitMuster;
		_delay_ms(2000);
		BitMuster = 0x55;
		PORTB = BitMuster;
 		_delay_ms(2000);
   }
}

