// Timer0 CompareA Interrupt toggled LED
// Hardware Arduino UNO

#include <avr/io.h>
#include <avr/interrupt.h>
#include "SpsMainLoop.h"


void InitInterrupt(void);
void InitInterrupt(void)
{
	// CS02,CS01,CS00 = 1,0,1  prescaler 1024 )
	// TIMSK0 = 2  compare A Interrupt Enable
	TCCR0A = (1 << WGM01);					// CTC-Mode
	TCCR0B = (1 << CS00) | (1 << CS02);		// Frequenz = 16MHz/1024/OCR0A
	TIMSK0 = (1 << OCIE0A);					// Interrupt, wenn compare A ist
	OCR0A = 156;							// Timer0 zählt von 0 bis OCR0A und löst dann einen Interrupt aus
											// 16MHz / 1024 / 156 = 100,16 Hz
	
	sei();	
}


int main (void)
{
	InitSPS();
	InitInterrupt();
	
	while(1) {;}
}


ISR(TIMER0_COMPA_vect)
{
	EndLessCounter++;
	SpsMainLoop();	
}



