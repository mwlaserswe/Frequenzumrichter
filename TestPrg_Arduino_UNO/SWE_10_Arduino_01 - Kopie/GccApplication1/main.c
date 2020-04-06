// Minimalprogramm mit dem Arduino UNO
// PortB.4 (Header Pin 12) und PortB.5 (Header Pin 13 bzw OnBoard LED) blinken


#include <avr/io.h>

// Tick counts Servo Output
#define ServoMin1 16
#define ServoMid1 22
#define ServoMax1 28
#define ServoMin2 16
#define ServoMid2 22
#define ServoMax2 28

// Tick counts Receiver Input
#define ReceiverMin 50			// 1100µs
#define ReceiverMid 80			// 1500µs
#define ReceiverMax 110			// 1900µs



volatile uint32_t cnt;
////volatile float EmpfMin;
////volatile float EmpfMax;
volatile float fReceiver;
////volatile float ServoMin;
////volatile float ServoMax;
volatile float fServo;
volatile float fM;
volatile float fT;

volatile uint8_t ServoPWM;
volatile uint8_t Result;


void CalcScaling(float fReceiverMin, float fReceiverMax, float fServoMin, float fServoMax)
{ 
	fM = (fServoMax - fServoMin) / (fReceiverMax - fReceiverMin);
	fT = fServoMax- (fM * fReceiverMax);
}




int main(void)
{
	//////float EmpfMin;
	//////float EmpfMax;
	//////float Empf;
	//////float ServoMin;
	//////float ServoMax;
	//////float Servo;
	//////float m;
	//////float t;
	
	CalcScaling(ReceiverMin, ReceiverMax, ServoMin2, ServoMax2);
	
	cnt = 5;
	
	ServoPWM = 0xEF;
	Result = ServoPWM & (1 << PINB4);
	
	fReceiver = 52;
	fServo = ((fM * fReceiver) + fT) + 0.5;
	
	ServoPWM = fServo;
	
	
	

	//DDRB = 0b11111111;		//DDRB in iom328p.h
	//DDRB = 0b00110000;		//DDRB in iom328p.h
	DDRB = (1 << PORTB4 | 1 << PORTB5);
	 
	while (1)
	{	PORTB = 0x30;		//PORTB in iom328p.h
		
		for (cnt=0; cnt<=50000; cnt++) {;}
		
		PORTB = 0x00;		//PORTB in iom328p.h
		
		for (cnt=0; cnt<=50000; cnt++) {;}

	}
	 

}
