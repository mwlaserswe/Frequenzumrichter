/*
 * SpsMainLoop.c
 *
 * Created: 17.12.2018 18:21:13
 *  Author: SWE
 */ 
#include <avr/io.h>

volatile uint32_t EndLessCounter;
volatile uint32_t DemoDelayTimer;
volatile uint32_t LifeBitTimer;

volatile uint8_t DemoState;
volatile uint8_t LifeBitState;

void SpsMainLoop (void)
	{
	// Statemachine "Demo"
	// Eine LED an PB6 blitzt im Sekundentakt kurz auf
	switch (DemoState)
	{
		case 0:
			PORTD = 0;
			DemoDelayTimer = EndLessCounter + 1000 / 10;	//1000 ms
			DemoState = 10;
			break;

		case 10:
			if (EndLessCounter > DemoDelayTimer)
			{
				PORTD = (1 << PIND6);
				DemoState = 20;
			}
			break;
			
		case 20:
			DemoState = 30;
			break;

		case 30:
			DemoState = 0;
			break;
}

	// Statemachine "LifeBit"
	// Die OnBoardLED blinkt mit 1Hz
	switch (LifeBitState)
	{
		case 0:
			LifeBitTimer = EndLessCounter + 500 / 10;	// 500 ms
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
}
