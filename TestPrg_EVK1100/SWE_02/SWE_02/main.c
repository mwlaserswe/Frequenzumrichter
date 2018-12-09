/*
 * SWE_02.c
 *
 * Created: 18.10.2018 22:03:58
 * Author : j1
 */ 

#include <avr/io.h>

int main(void)
{
    /* Replace with your application code */
	uint8_t counter;
	volatile uint8_t value;
	//int16_t x, yr;
	
	PORTE.DIR = 0xFF;
    
	while (1)
    {
		//counter++; 
		value=(1 << 2) | (1 << 5);
		//value &= 0b00001111;
		
		PORTE.OUT = value;
		
		//x=1;
		
		//yr=x - (x*x*x)/(1*2*3) + (x*x*x*x*x)/(1*2*3*4*5) - (x*x*x*x*x*x*x)/(1*2*3*4*5*6*7);
		//
		//PORTE.OUT = (int)(yr);
    }
}

