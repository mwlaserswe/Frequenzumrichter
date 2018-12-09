/*
 * SWE_02.c
 *
 * Created: 18.10.2018 22:03:58
 * Author : j1
 */ 

#include <avr/io.h>

volatile uint8_t tmp;

int main(void)
{
    /* Replace with your application code */


	
	PORTB.DIR = 0xFF;
    
	while (1)
    {

		tmp = PORTA.IN;
		tmp++;
		PORTB.OUT = tmp;
	
    }
}

