

#include <avr/io.h>

#define F_CPU 16000000
#define Button0 0
#define LED1 1

volatile uint32_t cnt;
#define DisplayDelay 50000


int main (void)
{	
	//////// Port B Bit 1 setzen
	//////DDRB = 0b00000010;		// Define Port B Bit 1 as Output
	//////
	//////while(1)
	//////{
		//////PORTB = 0x02;
	//////}
	
	
	//////// Port B Bit 0 auf Port B Bit weiterreichen
	//////DDRB = 0b00000010;		// Define Port B Bit 1 as Output
	//////PORTB = 0b00000001;		// Define Port B Bit 0 as pull up
		//////
	//////while (1)
	//////{	if (PINB & (1<<Button0))
		//////{
			//////PORTB |= (1<<LED1);		// Bit set
		//////}
		//////else
		//////{
			//////PORTB &= ~(1<<LED1);	// Bit reset (~ is complement)
		//////}
	//////}
	
	// Für PB1 ist Timer 1 zuständig. Beschreibung Pin PB1: OCR1A 
	// es gibt zwei TC1 Control Register: TCCR1A und TCCR1B
	//    WGM13,WGM12,WGM10,WGM11,WGM10 = 0,1,0,1    heißt Fast PWM 8-Bit
	//    COM1A1,COM1A0 = 1,0  heißt Clear OC1A on compare match, set OC1A at BOTTOM (Non-inverting mode)
	//    CS01,CS00 = 1,1      heißt clk/1 (no prescaling)
	TCCR1A = ((1 << COM1A1) | (1 << WGM10) );
	TCCR1B = ((1 << CS10) | ( 1<< WGM12));
	
	DDRB = 0b00000010;		// Define Port B Bit 1 as Output
	PORTB = 0b00000001;		// Define Port B Bit 0 as pull up
	
	while(1)
	{
		// duty cycle setzen
		OCR1A = 70;
		for (cnt=0; cnt<=DisplayDelay; cnt++) {;}
		
		// duty cycle setzen
		OCR1A = 200;
		for (cnt=0; cnt<=DisplayDelay; cnt++) {;}
	}

	
	
	
}

