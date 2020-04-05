/*
 * Demoprogramm für Input und Output ohne viel Drumrum
 *
 * Created: 29.03.2020 17:49:42
 * Author : Weing
 */ 

#include <avr/io.h>

#define LED1 5
#define But1 2


int main(void)
{

	DDRD = 0b00100000;		// Define Port D Bit 5 as Output
	PORTD = 0b00000100;		// Define Port D Bit 2 as pull up
	
	while (1)
	{	if (PIND & (1<<But1))
		{
			PORTD |= (1<<LED1);		// Bit set
		}
		else
		{
			PORTD &= ~(1<<LED1);	// Bit reset (~ is complement)
		}

	}
	

}
