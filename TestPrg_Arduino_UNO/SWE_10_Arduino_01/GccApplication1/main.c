// Minimalprogramm mit dem Arduino UNO
// PortB.4 (Header Pin 12) und PortB.5 (Header Pin 13 bzw OnBoard LED) blinken


#include <avr/io.h>

volatile uint32_t cnt;

int main(void)
{

	//DDRB = 0b11111111;		//DDRB in iom328p.h
	//DDRB = 0b00110000;		//DDRB in iom328p.h
	DDRB = (1 << PORTB4 | 1 << PORTB5);
	 
	while (1)
	{	PORTB = 0x30;		//PORTB in iom328p.h
		
		for (cnt=0; cnt<=50000; cnt++) {;}
		
		PORTB = 0x00;		//PORTB in iom328p.h
		
		for (cnt=0; cnt<=50000; cnt++) {;}

	}
	 

}
