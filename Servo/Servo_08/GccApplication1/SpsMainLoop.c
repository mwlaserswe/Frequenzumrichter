/*
 * SpsMainLoop.c
 *
 * Created: 17.12.2018 18:21:13
 *  Author: SWE
 */ 
#include <avr/io.h>

#define Servo1_LED_ON() PORTD |= (1<<PORTD5)
#define Servo1_LED_OFF() PORTD &= ~(1<<PORTD5)

volatile uint32_t EndLessCounter;

//////volatile uint32_t DemoDelayTimer;
//////volatile uint8_t DemoState;

volatile uint8_t LifeBitState;
volatile uint32_t LifeBitTimer;

volatile uint8_t Servo1State;
volatile uint8_t Servo1cnt;

volatile uint8_t Servo2State;
volatile uint8_t Servo2cnt;


void InitSPS (void);
void InitSPS (void)
{
	// Port B as Input			-> PB0 Input Servo 1 signal
	// Port PB5 as Output		-> Life Ticker
	DDRB |= (1 << PINB5);

	// Port PD5 as Output		-> check Servo 1 signal
	// Port PD6					-> check PLC clock
	// Port PD7 as Output		-> Indicator "time exceeds"
	DDRD |= (1 << PIND7) | (1 << PIND6) | (1 << PIND5);
	
	// Port PB3(OC2A) as Timer 2
	// es gibt zwei TC1 Control Register: TCCR2A und TCCR2B
	//    WGM22,WGM21,WGM20 = 0,1,1    heißt Fast PWM 8-Bit
	//    COM2A1,COM2A0 = 1,0  heißt Clear OC2A on compare match, set OC2A at BOTTOM (Non-inverting mode)
	//    CS22,CS21,CS20 = 1,1,1      heißt clk/1024 (From prescaler)
	//    Mode 3: Beschreibung: ATmega328_p data sheet.pdf Seite -206-
	TCCR2A |= ((1 << COM2A1) | (1 << WGM21) | (1 << WGM20) );
	TCCR2B |= ((1 << CS22) | (1 << CS21) | (1 << CS20));
		
	// Port  PB3(OC2A) as Output	-> Servo 1
	DDRB |= (1 << PINB3);

}


void SpsMainLoop (void);
void SpsMainLoop (void)
	{
	//////// Statemachine "Demo"
	//////// Eine LED an PB6 blitzt im Sekundentakt kurz auf
	//////switch (DemoState)
	//////{
		//////case 0:
			////////PORTD = 0;
			//////DemoDelayTimer = EndLessCounter + 1000 / 10;	//1000 ms
			//////DemoState = 10;
			//////break;
//////
		//////case 10:
			//////if (EndLessCounter > DemoDelayTimer)
			//////{
				////////PORTD = (1 << PIND6);
				//////DemoState = 20;
				//////DemoState = 0;
			//////}
			//////break;
			//////
		//////case 20:
			//////DemoState = 30;
			//////break;
//////
		//////case 30:
			//////DemoState = 0;
			//////break;
	//////}

	// Statemachine "LifeBit"
	// Die OnBoardLED blinkt mit 1Hz
	switch (LifeBitState)
	{
		case 0:
			LifeBitTimer = EndLessCounter + 50UL * 200UL;	// 200 ms at 50 kHz PLC clock
			LifeBitState = 10;
			break;

		case 10:
			if (EndLessCounter > LifeBitTimer)
			{
				PORTB ^= (1 << PINB5);	// Toggle on board LED
				LifeBitState = 0;
			}
			break;
	}
	

///		if ((PINB & 0x01) > 0)
///		{
///			Servo1_LED_OFF();
///		}
///		
///		if  ((PINB & 0x01) == 0)
///		{
///			Servo1_LED_ON();
///		}


	// State machine "Read Pulse Width From RC Receiver Channel 1"
	switch (Servo1State)
	{
		case 0:
		Servo1_LED_OFF();
		if ((PINB & 0x01) > 0)
		{
			Servo1cnt = 0;
			Servo1State = 10;
		}
		break;

		case 10:
		Servo1_LED_ON();
		if  ((PINB & 0x01) == 0)
		{
			
			if (Servo1cnt >= 95)
			{
				OCR2A = 28;
			}
			else if (Servo1cnt >= 65)
			{
				OCR2A = 22;
			}
			else
			{
				OCR2A = 16;
			}
			Servo1State = 0;
		}
		else
		{
			Servo1cnt += 1;
		}
		break;
	}
	

	// Statemachine "Read Pulse Width From RC Receicer Channel 1"
	// Die OnBoardLED blinkt mit 1Hz
	switch (Servo2State)
	{
		case 0:
		if (PIND == 2)
		{
			Servo2cnt = 0;
			Servo2State = 10;
		}
		break;

		case 10:
		if (PIND == 0)
		{
			Servo2State = 0;
		}
		else
		{
			Servo2cnt += 1;
		}
		break;
	}
	
		
}
