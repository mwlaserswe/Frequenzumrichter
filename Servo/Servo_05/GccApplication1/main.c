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
	
	PORTB = (1<<PINB0);				//Pull-up on ICP1
	
	TCCR1A = 0;						//Defaults
	
	//TCCR1B = 10x00101
	//Enable noise cancel, look for falling edge, Clock divided by 1024
	TCCR1B = (1<<ICNC1) | (1<<CS12) | (1<<CS10);

	TIMSK1 |= (1<<ICIE1);			//Input capture interrupt Enable
	
	sei();							//Enable interrupts
	
	uint16_t timediff;
	while(1){
		if(current_edge == 2){
			//Check for normal (no wrap-around)
			if (starting_cnt < ending_cnt){
				//A - B
				timediff = ending_cnt - starting_cnt;
				} else {
				//wrap around
				//(0xffff + A) - B, done without requiring signed math
				timediff = starting_cnt - ending_cnt;
				timediff = 0xffff - timediff;
			}
			if ((timediff > 10000) && (timediff < 30000)){
				LED_ON();	// Set Port D Bit 7
				_delay_ms(5000);
			}
			LED_OFF();  //Reset Port D Bit 7
			current_edge = 0;
		}
	}
}

ISR(TIMER1_CAPT_vect)
{
	if(current_edge == 0){
		
		//Save timestamp
		starting_cnt = ICR1;
		
		//Switch to rising edge
		TCCR1B |= (1<<ICES1);
		
		current_edge = 1;
		
		} else if (current_edge == 1){
		//Save timestamp
		ending_cnt = ICR1;
		
		//Switch to falling edge
		TCCR1B &= ~(1<<ICES1);
		
		current_edge = 2;
	}
	TIFR1 |= (1<<ICF1);
}
