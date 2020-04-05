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


#define LED7 7



int main(void)
{	int a;
    /* Replace with your application code */
	DDRD = (1<<PORTD7);
	
    while (1) 
    {
		BitMuster = 0xAA;
		PORTD = BitMuster;
		BitMuster = 0x55;
		PORTD = BitMuster;
		a = BitMuster + 3;
		if (a > 1)
			{ BitMuster = 0;
			}
  		if (a < 3)
  		{ BitMuster = 4;
  		}
		BitMuster = 0x55;
		PORTD = BitMuster;
	}
			
}

