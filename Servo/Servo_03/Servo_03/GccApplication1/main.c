

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED1 5
volatile uint8_t State;


void InitInterrupt (void)
{
	DDRB |= 0x00;

	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT0);

}


// Damit rufe ich den Interrupt auf:

ISR(PCINT0_vect)
{
	if (State == 0)
	{
		PORTD |= (1<<LED1);		// Bit set
		State = 1;
	}
	else
	{
		PORTD &= ~(1<<LED1);	// Bit reset (~ is complement)
		State = 0;
	}
}


int main (void)
{	DDRD = 0b00100000;		// Define Port D Bit 5 as Output
	PORTB = 0b00000001;		// Define Port B Bit 0 as pull up

	
	InitInterrupt();
	sei();

	while(1)
	{
		PORTD &= ~(1 << PD4);
	}
}

////////ATmega8, 16mHz,ein Kanal wird mit einem Ext.Int ausgewertet.
//////
//////#include <avr/io.h>
//////#include <avr/interrupt.h>
//////
//////volatile uint8_t Pulsdauer = 0;
//////
//////void (steigende_Flanke)(void);
//////void (fallende_Flanke)(void);
//////
//////SIGNAL(SIG_INTERRUPT0)
//////{
	//////if( MCUCR == 0x03) // Testen auf steigende Flanke
	//////{
		//////steigende_Flanke();
		//////return;
	//////}
//////
	//////if( MCUCR == 0x02 ) // Testen auf fallende Flanke
	//////{
		//////fallende_Flanke();
		//////return;
	//////}
//////}
//////void (steigende_Flanke)(void)
//////{
	//////TCNT0 = 0; // Timer Reset
	//////TCCR0 = (1<<CS02);  // Timer2 1/256 MCU start
	//////MCUCR = 0x02; // INT0 fallende Flanke
//////}
//////void (fallende_Flanke)(void)
//////{
	//////TCCR0 = 0x00; // Timer2 Stoppen
	//////Pulsdauer = TCNT0; // Timer Wert = Pulsdauer
	//////MCUCR = 0x03; // INT0 auf steigende Flanke
//////}
//////
//////int main(void) {
//////
	////////PD2 Int0 für RC Signal
	//////PORTD = 0x04;       // Pull up on PD2
	//////DDRD = 0xFB;        // PD2 in Rest out
//////
	////////Int0 steigende Flanke / INT0 enable
	//////MCUCR =(1<<ISC01) | (1<<ISC00);
	//////GICR =(1<<INT0);
//////
	//////sei();
//////
	//////for(;;)
	//////{
//////
		//////if (Pulsdauer < 68){"mach was"}
//////
		//////else if (Pulsdauer >= 68 && Pulsdauer < 85){"mach was anderes"}
//////
		//////else if (Pulsdauer >= 85 && Pulsdauer < 110){"mach was anderes"}
//////
		//////else if (Pulsdauer >= 110 && Pulsdauer < 150){"mach was anderes"}
	//////}
//////}




//////// https://www.mikrocontroller.net/attachment/6103/PWM.c 
//////#include <inttypes.h>
//////#include <avr/io.h>
//////#include <avr/interrupt.h>
////////#include <avr/signal.h>
////////#include <ioM8.h>
//////
//////
//////void
//////ioinit (void)
//////{
//////
	///////* PORTB als Ausgang */
	//////DDRB = (1<<PB7) | (1<<PB6) | (1<<PB5) | (1<<PB4)
	//////| (1<<PB3) | (1<<PB2) | (1<<PB1) | (1<<PB0) ;
//////
	///////* PORTB alle LEDs aus */
	//////PORTB = 0xFF ;
//////
	///////* PORTD PD2 als Eingang / Rest als Ausgang */
	//////DDRD = (1<<PD7) | (1<<PD6) | (1<<PD5) | (1<<PD4)
	//////| (1<<PD3) | ~(1<<PD2) | (1<<PD1) | (1<<PD0) ;
	//////
//////
	///////* Int0 steigende Flanke / INT0 enable */
	//////MCUCR = (1<<ISC01) | (1<<ISC00);
	//////GICR = (1<<INT0);
//////
	///////* Timer0 (8Bit) 1/8 MCU Takt */
	//////TCCR0 = (1<<CS01);
	//////
	///////* Timer1 16 Bit 1/8 MCU Takt */
	//////TCCR1B = (1<<CS11);
	//////
	///////* Timer2 8 Bit 1/8 MCU Takt */
	//////TCCR2 = (1<<CS21);
	//////
	///////* Timer1 Overflow */
	//////TIMSK = (1<<TOIE1);
	//////
	///////* enable interrupts */
	//////sei ();
//////}
//////
//////
//////int
//////main (void)
//////{
	//////ioinit ();
//////
	//////for (;;) ;
//////
	//////return (0);
//////}
//////
///////* Timer1 Überlauf für blinkende LEDs an PORTB */
//////SIGNAL(SIG_OVERFLOW1)
//////{
	//////static unsigned char Tick;
	//////Tick++;
//////
	//////if (Tick < 16)
	//////{
		//////PORTB = 0x00;
	//////}
	//////else if (Tick > 16)
	//////{
		//////PORTB = 0xFF;
	//////}
//////
	//////if (Tick > 31)
	//////{
		//////Tick = 0;
	//////}
	//////TCNT1 = 0x04;
//////}
//////
///////* INT0 */
//////SIGNAL(SIG_INTERRUPT0)
//////{
	//////TCNT0 = 0;
	//////TCNT2 = 0;
//////}






















//////// Das ist dieser Link:  https://www.mikrocontroller.net/attachment/6104/RC_SIGNAL4ok4LED.c
//////
//////
//////
///////* ATmega8 mit 8 Mhz */
///////* PORTB 4 LED's gegen VCC */
///////* RC-Signal an PD2 (INT0) */
///////* 1ms Pulsdauer = 31 */
///////* 1,5ms Pulsdauer = 46 */
///////* 2ms Pulsdauer = 62 */
//////
//////
//////#include <inttypes.h>
//////#include <avr/io.h>
//////#include <avr/interrupt.h>
////////#include <avr/signal.h>
////////#include <ioM8.h>
//////
//////#define MEINBIT0 0
//////#define MEINBIT1 1
//////#define MEINBIT2 2
//////#define MEINBIT3 3
//////#define MEINBIT4 4
//////#define MEINBIT5 5
//////#define MEINBIT6 6
//////#define MEINBIT7 7
//////
//////volatile uint8_t Pulsdauer = 0;
//////
//////void (steigende_Flanke)(void);
//////void (fallende_Flanke)(void);
//////
//////void
//////ioinit (void)
//////{
//////
	//////PORTB = 0xFF;
	//////DDRB = 0xFF;
	//////
	//////PORTD = 0x04;       // Pull up on PD2
	//////DDRD = 0xFB;        // PD2 in Rest out
	//////
	//////
	///////* Int0 steigende Flanke / INT0 enable */
	//////MCUCR = (1<<ISC01) | (1<<ISC00);
	//////GICR = (1<<INT0);
	//////
	///////* enable interrupts */
	//////sei ();
//////}
//////
//////int
//////main (void)
//////{
	//////ioinit ();
//////
	//////for (;;)
	//////{
//////
		//////if (Pulsdauer < 32)
		//////{
			//////PORTB = 0xFF;
		//////}
		//////
		//////else if (Pulsdauer > 32 && Pulsdauer < 41)
		//////{
			//////PORTB = 0xFE; // LED PB0 an invertiert, da STK500 gegen VCC
		//////}
		//////
		//////else if (Pulsdauer > 41 && Pulsdauer < 50)
		//////{
			//////PORTB = 0xFC; //LED PB1 und PB0 an, invertiert, da STK500 gegen VCC
		//////}
		//////
		//////else if (Pulsdauer > 50 && Pulsdauer < 59)
		//////{
			//////PORTB = 0xF8; //LED PB2,PB1 und PB0 an, invertiert, da STK500 gegen VCC
		//////}
		//////
		//////else if (Pulsdauer > 59 && Pulsdauer < 68)
		//////{
			//////PORTB = 0xF0; //LED PB3,PB2,PB1 und PB0 an, invertiert, da STK500 gegen VCC
		//////}
		//////
	//////}
	//////;
//////
	//////return (0);
//////}
//////
//////SIGNAL(SIG_INTERRUPT0)
//////{
	//////if( MCUCR == 0x03) // Testen auf steigende Flanke
	//////{
		//////steigende_Flanke();
		//////return;
	//////}
	//////
	//////if( MCUCR == 0x02 ) // Testen auf fallende Flanke
	//////{
		//////fallende_Flanke();
		//////return;
	//////}
//////}
//////void (steigende_Flanke)(void)
//////{
	//////TCNT1 = 0; // Timer Reset
	//////TCCR1B = (1<<CS12);  // Timer1 1/256 MCU start
	//////MCUCR = 0x02; // INT0 fallende Flanke
//////}
//////void (fallende_Flanke)(void)
//////{
	//////TCCR1B = 0x00; // Timer2 Stoppen
	//////Pulsdauer = TCNT1; // Timer Wert = Pulsdauer
	//////MCUCR = 0x03; // INT0 auf steigende Flanke
//////}
