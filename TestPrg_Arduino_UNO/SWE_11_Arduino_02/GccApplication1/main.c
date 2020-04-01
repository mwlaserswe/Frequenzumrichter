// PWM an Port PD6
// Hardware Arduino UNO


#include <avr/io.h>

volatile uint32_t cnt;
#define DisplayDelay 50000

int main (void)
{
	// 8-Bit PWM
	// F¸r PD6 ist Timer 0 zust‰ndig
	// es gibt zwei TC0 Control Register: TCCR0A und TCCR0B
	//    WGM01,WGM00 = 1,1    heiﬂt Fast PWM
	//    COM0A1,COM0A0 = 1,0  heiﬂt Clear OC0A on compare match, set OC0A at BOTTOM (Non-inverting mode)
	//    CS01,CS00 = 1,1      heiﬂt clk/1 (no prescaling)
	TCCR0A = ((1 << COM0A1) | (1 << WGM01) | (1 << WGM00) );
	TCCR0B = ((1 << CS10) | (1 << CS00));
	
	// Port PD6 auf Output setzen
	DDRD |= (1 << PIND6);
	
		
	while(1)
	{
		// duty cycle setzen
		OCR0A = 70;
		for (cnt=0; cnt<=DisplayDelay; cnt++) {;}
			
		// duty cycle setzen
		OCR0A = 200;
		for (cnt=0; cnt<=DisplayDelay; cnt++) {;}
	}

	

}
