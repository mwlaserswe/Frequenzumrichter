/*
 * SpsMainLoop.h
 *
 * Created: 17.12.2018 18:33:29
 *  Author: SWE
 */ 

volatile uint32_t EndLessCounter;
volatile uint8_t GlbServo1Cnt;
volatile uint8_t GlbServo2Cnt;



void InitSPS (void);
void SpsMainLoop (void);
void PLC_StandardTask (void);

