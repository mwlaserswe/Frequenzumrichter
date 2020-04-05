/*
 * PLC_StandardTask.c
 *
 * Created: 05.04.2020 21:57:49
 *  Author: SWE
 */ 

#include <avr/io.h>
#include "SpsMainLoop.h"

#define StateOff			0
#define StateFoldOut		1
#define StateOut			2
#define StateRampUpMotor	3
#define StateMotorOn		4
#define StateRampDownMotor	5
#define StateMotorOff		6
#define StateFoldIn			7

#define Servo1_LED_ON() PORTD |= (1<<PORTD5)
#define Servo1_LED_OFF() PORTD &= ~(1<<PORTD5)


 
volatile uint8_t LifeBitState;
//volatile uint32_t LifeBitTimer;
volatile uint32_t TON_LifeBit;

volatile uint8_t GenaralFunctionState;
volatile uint8_t PWM_Servo1;
volatile uint32_t TON_PWM_Servo1;

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
	
	/*
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
	*/

	// General function state machine
	// FoldOut / FoldIN
	// Motor On / Off
	switch (GenaralFunctionState)
	{
		
		case StateOff:
			PWM_Servo1 = 16;
			TON_PWM_Servo1 = 0;
			
			if (GlbServo1Cnt >= 65)
			{
				GenaralFunctionState = StateFoldOut;
			}
			break;
			
		case StateFoldOut:
			TON_PWM_Servo1 += 1;
			if (TON_PWM_Servo1 >= 20)	// increase PWM every 20 ms
			{	
				TON_PWM_Servo1 = 0;
				PWM_Servo1 += 1;
				if (PWM_Servo1 >= 28)
				 {
					GenaralFunctionState = StateOut;					 
				 }
			}
			
			if (GlbServo1Cnt < 65)
			{
				GenaralFunctionState = StateFoldIn;
			}
			break;
			
			
			
		case StateOut:
			PWM_Servo1 = 28;
			if (GlbServo1Cnt < 65)
			{
				GenaralFunctionState = StateFoldIn;
			}


			if (GlbServo1Cnt > 95)
			{
				GenaralFunctionState = StateRampUpMotor;
			}


			break;
		


		case StateRampUpMotor:
			GenaralFunctionState = StateMotorOn;
			break;

		case StateMotorOn:
			Servo1_LED_ON();
			if (GlbServo1Cnt < 95)
			{
				GenaralFunctionState = StateRampDownMotor;
			}
		
		
		break;

		case StateRampDownMotor:
			Servo1_LED_OFF();
			if (GlbServo1Cnt < 65)
			{
				GenaralFunctionState = StateFoldIn;
				TON_PWM_Servo1 = 0;
			}

			if (GlbServo1Cnt > 95)
			{
				GenaralFunctionState = StateRampUpMotor;
				TON_PWM_Servo1 = 0;
			}

			break;

		case StateFoldIn:
			TON_PWM_Servo1 += 1;
			if (TON_PWM_Servo1 >= 20)	// decrease PWM every 20 ms
			{
				TON_PWM_Servo1 = 0;
				PWM_Servo1 -= 1;
				if (PWM_Servo1 <= 16)
				{
					GenaralFunctionState = StateOff;
				}
			}
			
			if (GlbServo1Cnt > 65)
			{
				GenaralFunctionState = StateFoldOut;
			}
			break;
			

			break;
		break;
		
		default:
			GenaralFunctionState = StateOff;
		


	}
	
	
	OCR2A = PWM_Servo1;
}
