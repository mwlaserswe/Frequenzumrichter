/*
 * PLC_StandardTask.c
 *
 * Created: 05.04.2020 21:57:49
 *  Author: SWE
 */ 

#include <avr/io.h>
#include "SpsMainLoop.h"

volatile uint8_t LifeBitState;
volatile uint32_t LifeBitTimer;

volatile uint32_t TON_LifeBit;

void PLC_StandardTask (void);
void PLC_StandardTask (void)
{
	// State machine "LifeBit"
	// Die OnBoardLED blinking with 200 ms
	switch (LifeBitState)
	{
		case 0:
		TON_LifeBit = 0;	
		LifeBitState = 10;
		break;

		case 10:
		TON_LifeBit += 1;
		if (TON_LifeBit >= 200)		// 200 ms at 50 kHz PLC clock
		{
			PORTB ^= (1 << PINB5);	// Toggle on board LED
			LifeBitState = 0;
		}
		break;
	}
	
	
	// Set Servo1 PWM	
	if (GlbServo1Cnt >= 95)
	{
		OCR2A = 28;
	}
	else if (GlbServo1Cnt >= 65)
	{
		OCR2A = 22;
	}
	else
	{
		OCR2A = 16;
	}

			
	// Set Servo1 PWM	
	if (GlbServo2Cnt >= 95)
	{
		OCR0A = 28;
	}
	else if (GlbServo2Cnt >= 65)
	{
		OCR0A = 22;
	}
	else
	{
		OCR0A = 16;
	}

			
}
