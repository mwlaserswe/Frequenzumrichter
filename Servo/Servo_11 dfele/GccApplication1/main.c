// Timer0 CompareA Interrupt toggled LED
// Hardware Arduino UNO
// 
// ToDo:
// - F_CPU muﬂ 16000000UL sein
// - wie lang dauert der PLC Standard Task

#include <avr/io.h>
#include <avr/interrupt.h>
#include "SpsMainLoop.h"

#define LED_ON() PORTD |= (1<<PORTD7)
#define LED_OFF() PORTD &= ~(1<<PORTD7)

volatile uint8_t StandardTaskCnt;
volatile uint8_t StandardTaskTrigger;

void InitInterrupt(void);
void InitInterrupt(void)
{
	/* Timer 0 as PLC clock
	// Init PLC clock
	TCCR0A |= (1 << WGM01) | (1 << COM0A0);	// CTC-Mode, Signal output at PD6
	TCCR0B |= (1 << CS01);					// CS02,CS01,CS00 = 0,1,0  Prescaler 8, Frequenz = 16MHz/8/OCR0A=160  => 100 kHz  
	TIMSK0 |= (1 << OCIE0A);					// Interrupt, wenn compare A ist
	OCR0A = 39;								// Timer0 z‰hlt von 0 bis OCR0A und lˆst dann einen Interrupt aus
											// f(PD6) = f(clk) / (2 * N *(1 + OCR0A))
											// f(PD6) = 16MHz / (2 * 8 *(1 + 39)) = 25 kHz
											// ==> PLC clock = 50 kHz
	*/
	
	// Timer 1 as PCL clock
	// Init PLC clock
	TCCR1A = 0;	// CTC-Mode, no Signal output at Pin
	TCCR1B |= (1 << CS11) | (1 << WGM12);					// CS12,CS11,CS10 = 0,1,0  Prescaler 8, Frequenz = 16MHz/8/OCR1A=160  => 100 kHz
	TIMSK1 |= (1 << OCIE1A);					// Interrupt, wenn compare A ist
	OCR1A = 39;								// Timer0 z‰hlt von 0 bis OCR0A und lˆst dann einen Interrupt aus
	// f(PD6) = f(clk) / (2 * N *(1 + OCR0A))
	// f(PD6) = 16MHz / (2 * 8 *(1 + 39)) = 25 kHz
	// ==> PLC clock = 50 kHz


	sei();	
}


int main (void)
{
	InitSPS();
	InitInterrupt();
	
	while(1) 
	{
		if (StandardTaskTrigger == 1)
		{		
			StandardTaskTrigger = 0;
			PLC_StandardTask();
		}
		
	}
}

/* Timer 0 as PLC clock
ISR(TIMER0_COMPA_vect)
*/

ISR(TIMER1_COMPA_vect)
{	LED_ON();				// Led checks "time exceeds"
	EndLessCounter++;
	
	if (StandardTaskCnt >= 50)
	{
		StandardTaskTrigger = 1;
		StandardTaskCnt = 0;
	} 
	else
	{
		StandardTaskCnt += 1;
	}
	
	SpsMainLoop();
	LED_OFF();				// Led checks "time exceeds"
}



