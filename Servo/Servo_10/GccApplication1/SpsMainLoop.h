/*
 * SpsMainLoop.h
 *
 * Created: 17.12.2018 18:33:29
 *  Author: SWE
 */ 

volatile uint32_t EndLessCounter;
volatile uint8_t GlbServo1Cnt;
volatile uint8_t GlbServo2Cnt;

// Tick counts from RC receiver
#define ThresholdFoldOut 65
#define ThresholdMotorOn 95

// Tick counts Servo Output
#define ServoMin1 16
#define ServoMid1 22
#define ServoMax1 28
#define ServoMin2 16
#define ServoMid2 22
#define ServoMax2 28


void InitSPS (void);
void SpsMainLoop (void);
void PLC_StandardTask (void);

