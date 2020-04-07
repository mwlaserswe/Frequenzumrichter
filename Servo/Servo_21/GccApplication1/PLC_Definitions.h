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
#define ReceiverMin 50			// 1100탎
#define ThresholdFoldOut 65
#define ReceiverMid 80			// 1500탎
#define ThresholdMotorOn 95
#define ReceiverMax 110			// 1900탎
#define ReceiverTooHigh 150	

// Tick counts Servo Output
#define ServoMin1 2199			// 1100탎
#define ServoMid1 2999			// 1500탎
#define ServoMax1 3799			// 1900탎
#define ServoMin2 16			// 1100탎
#define ServoMid2 22			// 1500탎
#define ServoMax2 28			// 1900탎






void InitPLC (void);
void PLC_FastTask (void);
void PLC_StandardTask (void);

