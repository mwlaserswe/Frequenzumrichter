// Timer0 Overflow Interrupt toggled LED
// Hardware Arduino UNO

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED13 5

volatile uint32_t cnt;
#define DisplayDelay 50000

int main (void)
{
	// Port PB5 auf Output setzen
	DDRB |= (1 << PINB5);
	// PORTB = 0x20;
	
	// CS02,CS01,CS00 = 1,0,1  prescaler 1024 )
	// TIMSK0 = 1  Overflow Interrupt Enable
	TCCR0B = (1 << CS00) | (1 << CS02);		//16MHz/1024/256	= 30Hz
	TIMSK0 = (1 << TOIE0);
	
	sei();
	
	while(1) {;}
}


ISR(TIMER0_OVF_vect)
{
	PORTB ^= (1 << PINB5);	// Toggle on board LED
}



