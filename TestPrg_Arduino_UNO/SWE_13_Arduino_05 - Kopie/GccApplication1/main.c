/*
 * Demoprogramm für Input und Output ohne viel Drumrum
 *
 * Created: 29.03.2020 17:49:42
 * Author : Weing
 */ 

#include <avr/io.h>

#define LED1 5
#define But1 0


int main(void)
{

	DDRD = 0b00100000;		// Define Port D Bit 5 as Output
	
	DDRB = 0b00000000;		// Define Port B as Input
	PORTB = 0b00000000;		// Define Port B no pull up
	
	while (1)
	{	if (PINB & (1<<But1))
		{
			PORTD |= (1<<LED1);		// Bit set
		}
		else
		{
			PORTD &= ~(1<<LED1);	// Bit reset (~ is complement)
		}

	}
	

}
