/*
 * PLC_StandardTask.c
 *
 * Created: 05.04.2020 21:57:49
 *  Author: SWE
 */ 

#include <avr/io.h>
#include "PLC_Definitions.h"

#define StateOff			0
#define StateFoldOut		1
#define StateOut			2
#define StateRampUpMotor	3
#define StateMotorOn		4
#define StateRampDownMotor	5
#define StateMotorOff		6
#define StateFoldIn			7


#define StateTstOff			0
#define StateTstFwd			1
#define StateTestRev		2




volatile uint8_t CurrentMotorCmdOld;
volatile uint8_t LifeBitState;
volatile uint32_t TON_LifeBit;

volatile uint8_t WatchdogState;
volatile uint32_t TON_Watchdog;

volatile uint8_t GenaralFunctionState;
volatile uint16_t PWM_Servo1;
volatile uint8_t PWM_Servo2;
volatile uint32_t TON_PWM_Servo;

volatile uint8_t TestSequenceState;
volatile uint32_t TON_TestSequenz;




void PLC_StandardTask (void);
void PLC_StandardTask (void)
{
	float fCurrentMotorCmd;
	uint8_t CurrentMotorCmd;


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
	

	if ((GlbServo1Cnt > ReceiverTooLow) && (GlbServo2Cnt > ReceiverTooLow) && (GlbServo1Cnt < ReceiverTooHigh) && (GlbServo2Cnt < ReceiverTooHigh))
	{
		DDRB |= (1 << PINB2);
		DDRB |= (1 << PINB3);	
	}



	// State machine "Watchdog"
	// If 
	switch (WatchdogState)
	{
		case 0:
			TON_Watchdog = 0;
			WatchdogState = 10;
			break;

		case 10:
			TON_Watchdog += 1;
			if (Watchdog1 && Watchdog2)
			{
				Watchdog1 = 0;					// clear Watchdog flag
				Watchdog2 = 0;					// clear Watchdog flag
				WatchdogState = 0;
			}
			else if (TON_Watchdog >= 500)		// No signal from RC Receiver for 500 ms
			{
				GlbServo1Cnt = ReceiverMin;
				GlbServo2Cnt = ReceiverMin;
				WatchdogState = 0;
			}
			break;
	}
	
	

	
	

	fCurrentMotorCmd = ((fM * GlbServo2Cnt) + fT) + 0.5;
	CurrentMotorCmd = fCurrentMotorCmd;
	
	// Hysteresis to avoid jitter
	if (CurrentMotorCmd == CurrentMotorCmdOld -1 )
	{
		CurrentMotorCmd = CurrentMotorCmdOld;
	}
	else
	{
		CurrentMotorCmdOld = CurrentMotorCmd;
	}

	// General function state machine
	// FoldOut / FoldIN
	// Motor On / Off
	switch (GenaralFunctionState)
	{
		
		case StateOff:
			PWM_Servo1 = ServoMin1;
			PWM_Servo2 = ServoMin2;
		
			if (GlbServo1Cnt >= ThresholdFoldOut)
			{
				GenaralFunctionState = StateFoldOut;
			}
			break;
			
		case StateFoldOut:
			PWM_Servo2 = ServoMin2;
			
			PWM_Servo1 += 1;
			if (PWM_Servo1 >= ServoMax1)
			{
				GenaralFunctionState = StateOut;
			}
			
			if (GlbServo1Cnt < ThresholdFoldOut)
			{
				GenaralFunctionState = StateFoldIn;
			}
			break;
			
			
			
		case StateOut:
			PWM_Servo2 = ServoMin2;
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
			PWM_Servo1 = ServoMax1;
			
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 20)	// increase PWM every 10 ms
			{
				TON_PWM_Servo = 0;
				PWM_Servo2 += 1;
				if (PWM_Servo2 >= CurrentMotorCmd)
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
			PWM_Servo1 = ServoMax1;
			PWM_Servo2 = CurrentMotorCmd;
			
			if (GlbServo1Cnt < ThresholdMotorOn)
			{
				GenaralFunctionState = StateRampDownMotor;
			}
		
		
		break;

		case StateRampDownMotor:
			PWM_Servo1 = ServoMax1;
			
			TON_PWM_Servo += 1;
			if (TON_PWM_Servo >= 20)	// increase PWM every 10 ms
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
			PWM_Servo2 = ServoMin2;

			PWM_Servo1 -= 1;
			if (PWM_Servo1 <= ServoMin1)
			{
				GenaralFunctionState = StateOff;
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
	
	


	OCR1B = PWM_Servo1;
	OCR2A = PWM_Servo2;

}
