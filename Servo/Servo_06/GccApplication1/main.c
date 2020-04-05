#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define F_CPU 16000000
#include <util/delay.h>
#define LED_ON() PORTD |= 1<<7
#define LED_OFF() PORTD &= ~(1<<7)
volatile unsigned char current_edge = 0;
volatile uint16_t starting_cnt;
volatile uint16_t ending_cnt;

int main(void)
{
	//LED as output
	DDRD = (1<<PIND7);
	

	DDRB = 0b00000010;				// Define Port B Bit 1 as Output
	PORTB = 0;						//kein Pull-up on ICP1. Servo liefert highaktives Signal
	
	TCCR1A = 0;						//Defaults
	
	/* Init Pin B0 Interrupt */
	PCICR = (1<<PCIE0);				//Die Pins 0 bis 7 werden als Interrupts hergenommen
	PCMSK0 = (1<<PCINT0);			//Hier werden die Interrupts für die jeweiligen Pins enabled
	EICRA = (1<<ISC01);				//10: fallende Flanke       11: steigende Flanke
	
	sei();							//Enable interrupts
	
	
	
		// Für PB1 ist Timer 1 zuständig. Beschreibung Pin PB1: OCR1A
		// es gibt zwei TC1 Control Register: TCCR1A und TCCR1B
		//    WGM13,WGM12,WGM10,WGM11,WGM10 = 0,0,0,0    heißt Normal Mode
		//    COM1A1,COM1A0 = 1,0  heißt Clear OC1A on compare match, set OC1A at BOTTOM (Non-inverting mode)
		//    CS12,CS11,CS10 = 0,0,1      heißt clk/1 (no prescaling)
		//    Mode 0: Beschreibung: ATmega328_p data sheet.pdf Seite -176-

		TCCR1A = 0;	// Mode 0
		TCCR1B = 0; // Timer 1 stopped
		

	
	
	
	
	
	
	
	uint16_t timediff;
	while(1){;
		//////if(current_edge == 2){
			////////Check for normal (no wrap-around)
			//////if (starting_cnt < ending_cnt){
				////////A - B
				//////timediff = ending_cnt - starting_cnt;
				//////} else {
				////////wrap around
				////////(0xffff + A) - B, done without requiring signed math
				//////timediff = starting_cnt - ending_cnt;
				//////timediff = 0xffff - timediff;
			//////}
			//////if ((timediff > 1000) && (timediff < 30000)){
				//////LED_ON();	// Set Port D Bit 7
				//////_delay_ms(1000);
			//////}
			//////LED_OFF();  //Reset Port D Bit 7
			//////current_edge = 0;
		//////}
	}
}


ISR(PCINT0_vect) /* Pin Change Interrupt Request 0 */
{	
	if (PORTD == 0)
	{
		LED_ON();
		TCCR1B =  (1<<CS12); // Prescaler 256 

	}
	else
	{
		LED_OFF();
		TCCR1B = 0; // Timer 1 stopped

	}
	
	
}
