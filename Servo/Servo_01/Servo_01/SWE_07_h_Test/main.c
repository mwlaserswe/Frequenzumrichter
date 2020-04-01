/*
 * Servo_01
 *
 * Nur mal schauen. ob die gelbe LED auf der Arduino UNO Platine im 200 ms Takt blinkt.
 * Die gelbe LED ist an Port B5, Pin 19 des ATMEGA328-Chips, Pin 13 der Arduino UNO Platine
 *
 * Created: 28.11.2018 13:56:57
 * Author : j1
 */ 

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

volatile uint8_t BitMuster;

int main(void)
{
    /* Replace with your application code */
	DDRB = 0xFF;
	
    while (1) 
    {
		BitMuster = 0xAA;
		PORTB = BitMuster;
		_delay_ms(200);
		BitMuster = 0x55;
		PORTB = BitMuster;
 		_delay_ms(200);
   }
}

