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
volatile uint8_t PWM_Servo2;
volatile uint32_t TON_PWM_Servo;

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
	if (GlbServo1Cnt >= ThresholdMotorOn)
	{
		OCR2A = ServoMax1;
	}
	else if (GlbServo1Cnt >= ThresholdFoldOut)
	{
		OCR2A = ServoMid1;
	}
	else
	{
		OCR2A = ServoMin1;
	}

			
	// Set Servo2 PWM	
	if (GlbServo2Cnt >= ThresholdMotorOn)
	{
		OCR0A = ServoMax2;
	}
	else if (GlbServo2Cnt >= ThresholdFoldOut)
	{
		OCR0A = ServoMid1;
	}
	else
	{
		OCR0A = ServoMin2;
	}
	*/

	// General function state machine
	// FoldOut / FoldIN
	// Motor On / Off
	switch (GenaralFunctionState)
	{
		
		case StateOff:
			PWM_Servo1 = ServoMin1;
			TON_PWM_Servo = 0;
			
			if (GlbServo1Cnt >= ThresholdFoldOut)
			{
				GenaralFunctionState = StateFoldOut;
			}
			break;
			
		case StateFoldOut:
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 200)	// increase PWM every 20 ms
			{	
				TON_PWM_Servo = 0;
				PWM_Servo1 += 1;
				if (PWM_Servo1 >= ServoMax1)
				 {
					GenaralFunctionState = StateOut;					 
				 }
			}
			
			if (GlbServo1Cnt < ThresholdFoldOut)
			{
				GenaralFunctionState = StateFoldIn;
			}
			break;
			
			
			
		case StateOut:
			PWM_Servo1 = ServoMax1;
			if (GlbServo1Cnt < ThresholdFoldOut)
			{
				GenaralFunctionState = StateFoldIn;
			}


			if (GlbServo1Cnt > ThresholdMotorOn)
			{
				GenaralFunctionState = StateRampUpMotor;
			}


			break;
		


		case StateRampUpMotor:
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 200)	// increase PWM every 20 ms
			{
				TON_PWM_Servo = 0;
				PWM_Servo2 += 1;
				if (PWM_Servo2 >= ServoMax2)
				{
					GenaralFunctionState = StateMotorOn;
				}
			}
			
			if (GlbServo1Cnt < ThresholdMotorOn)
			{
				GenaralFunctionState = StateOut;
			}
			break;
			
			
		case StateMotorOn:
			PWM_Servo2 = ServoMax2;
			Servo1_LED_ON();
			TON_PWM_Servo = 0;

			if (GlbServo1Cnt < ThresholdMotorOn)
			{
				GenaralFunctionState = StateRampDownMotor;
			}
		
		
		break;

		case StateRampDownMotor:
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 200)	// increase PWM every 20 ms
			{
				TON_PWM_Servo = 0;
				PWM_Servo2 -= 1;
				if (PWM_Servo2 <= ServoMin2)
				{
					GenaralFunctionState = StateFoldIn;
				}
			}
			
			if (GlbServo1Cnt > ThresholdMotorOn)
			{
				GenaralFunctionState = StateRampUpMotor;
			}
			break;


		case StateFoldIn:
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 200)	// decrease PWM every 20 ms
			{
				TON_PWM_Servo = 0;
				PWM_Servo1 -= 1;
				if (PWM_Servo1 <= ServoMin1)
				{
					GenaralFunctionState = StateOff;
				}
			}
			
			if (GlbServo1Cnt > ThresholdFoldOut)
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
	OCR0A = PWM_Servo2;
}
