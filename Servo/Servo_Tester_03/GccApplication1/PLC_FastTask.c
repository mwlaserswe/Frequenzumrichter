/*
 * SpsMainLoop.c
 *
 * Created: 17.12.2018 18:21:13
 *  Author: SWE
 */ 
#include <avr/io.h>
#include "PLC_Definitions.h"

#define Servo1_LED_ON() PORTD |= (1<<PORTD5)
#define Servo1_LED_OFF() PORTD &= ~(1<<PORTD5)

volatile uint8_t Servo1State;
volatile uint8_t Servo1cnt;

volatile uint8_t Servo2State;
volatile uint8_t Servo2cnt;



void CalcScaling(float fReceiverMin, float fReceiverMax, float fServoMin, float fServoMax);
void CalcScaling(float fReceiverMin, float fReceiverMax, float fServoMin, float fServoMax)
{
	fM = (fServoMax - fServoMin) / (fReceiverMax - fReceiverMin);
	fT = fServoMax- (fM * fReceiverMax);
}



void InitPLC (void);
void InitPLC (void)
{
	// Port B as Input			-> PB0 Input Servo 1 signal
	// Port PB5 as Output		-> Life Ticker
	DDRB |= (1 << PINB5);

	// Port PD5 as Output		-> check Servo 1 signal
	// Port PD7 as Output		-> Indicator "time exceeds"
	DDRD |= (1 << PIND7) | (1 << PIND5);
	


	// =================== Servo 1 Output ===================
	// F¸r PB2 ist Timer 1 zust‰ndig. Schreiben auf Pin PB2: OCR1B
	// es gibt zwei TC1 Control Register: TCCR1A und TCCR1B
	//    WGM13,WGM12,WGM10,WGM11,WGM10 = 1,1,1,1    heiﬂt Fast PWM 16-Bit
	//    Es von 0 bis OCR1A gez‰hlt. -> OCR1A = 0x7FFF
	//    COM1B1,COM1B0 = 1,0  heiﬂt Clear OC1B on compare match, set OC1B at BOTTOM (Non-inverting mode)
	//    CS12,CS11,CS10 = 0,1,0      heiﬂt clk/8 (From prescaler)
	//    Mode 15: Beschreibung: ATmega328_p data sheet.pdf Seite -176-
	TCCR1A = ((1 << COM1B1) | (1 << WGM11) | (1 << WGM10) );
	TCCR1B = (( 1 << WGM13) | ( 1<< WGM12) | (1 << CS11));
	OCR1A = 0x7FFF;


	DDRB |= (1 << PINB2);
		

	// =================== Servo 2 Output ===================
	// Port PB3(OC2A) as Timer 2
	// es gibt zwei TC1 Control Register: TCCR2A und TCCR2B
	//    WGM22,WGM21,WGM20 = 0,1,1    heiﬂt Fast PWM 8-Bit
	//    COM2A1,COM2A0 = 1,0  heiﬂt Clear OC2A on compare match, set OC2A at BOTTOM (Non-inverting mode)
	//    CS22,CS21,CS20 = 1,1,1      heiﬂt clk/1024 (From prescaler)
	//    Mode 3: Beschreibung: ATmega328_p data sheet.pdf Seite -206-
	TCCR2A |= ((1 << COM2A1) | (1 << WGM21) | (1 << WGM20) );
	TCCR2B |= ((1 << CS22) | (1 << CS21) | (1 << CS20));

	// Port  PB3(OC2A) as Output	-> Servo 1
	DDRB |= (1 << PINB3);
	

/*
	// =================== Servo xx Output ===================
	// Port PB3(OC0A) as Timer 2
	// es gibt zwei TC1 Control Register: TCCR0A und TCCR0B
	//    WGM02,WGM01,WGM00 = 0,1,1    heiﬂt Fast PWM 8-Bit
	//    COM0A1,COM0A0 = 1,0  heiﬂt Clear OC0A on compare match, set OC0A at BOTTOM (Non-inverting mode)
	//    CS02,CS01,CS00 = 1,0,1      heiﬂt clk/1024 (From prescaler)
	//    Mode 3: Beschreibung: ATmega328_p data sheet.pdf Seite -xxx-
	TCCR0A |= ((1 << COM0A1) | (1 << WGM01) | (1 << WGM00) );
	TCCR0B |= ((1 << CS02) | (1 << CS00));
	
	// Port  PB3(OC0A) as Output	-> Servo 2
	DDRD |= (1 << PIND6);
*/



	CalcScaling(ReceiverMin, ReceiverMax, ServoMin2, ServoMax2);

}


void PLC_FastTask (void);
void PLC_FastTask (void)
{


	// State machine "Read Pulse Width From RC Receiver Channel 1"
	switch (Servo1State)
	{
		case 0:
		//Servo1_LED_OFF();
		if ((PIND & 0x04) > 0)
		{
			Servo1cnt = 0;
			Servo1State = 10;
		}
		break;

		case 10:
		//Servo1_LED_ON();
		if  ((PIND & 0x04) == 0)
		{ 
			GlbServo1Cnt = Servo1cnt;
			Watchdog = 1;
			Servo1State = 0;
		}
		else
		{
			Servo1cnt += 1;
		}
		break;
	}
	

	// State machine "Read Pulse Width From RC Receiver Channel 2"
	switch (Servo2State)
	{
		case 0:
		Servo1_LED_OFF();
		if ((PIND & 0x10) > 0)
		{
			Servo2cnt = 0;
			Servo2State = 10;
		}
		break;

		case 10:
		Servo1_LED_ON();
		if  ((PIND & 0x10) == 0)
		{
			GlbServo2Cnt = Servo2cnt;
			Watchdog = 1;
			Servo2State = 0;
		}
		else
		{
			Servo2cnt += 1;
		}
		break;
	}

		
}
