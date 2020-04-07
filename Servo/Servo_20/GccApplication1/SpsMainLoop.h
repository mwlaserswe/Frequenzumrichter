/*
 * SpsMainLoop.h
 *
 * Created: 17.12.2018 18:33:29
 *  Author: SWE
 */ 

volatile uint8_t Watchdog1;
volatile uint8_t Watchdog2;
volatile uint8_t GlbServo1Cnt;
volatile uint8_t GlbServo2Cnt;

volatile float fEmpf;
volatile float fServo;
volatile float fM;
volatile float fT;

// Tick counts from RC receiver

#define ReceiverTooLow 20
#define ReceiverMin 50			// 1100�s
#define ThresholdFoldOut 65
#define ReceiverMid 80			// 1500�s
#define ThresholdMotorOn 95
#define ReceiverMax 110			// 1900�s
#define ReceiverTooHigh 150	

// Tick counts Servo Output
#define ServoMin1 68			// 1100�s
#define ServoMid1 93			// 1500�s
#define ServoMax1 118			// 1900�s
#define ServoMin2 16			// 1100�s
#define ServoMid2 22			// 1500�s
#define ServoMax2 28			// 1900�s






void InitSPS (void);
void SpsMainLoop (void);
void PLC_StandardTask (void);

