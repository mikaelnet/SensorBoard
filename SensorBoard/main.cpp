/*
 * SensorBoard.cpp
 *
 * Created: 2013-12-27 22:14:11
 *  Author: mikael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "core/cpu.h"

void setup()
{
#if F_CPU == 32000000UL
	cpu_set_32_MHz();
#endif

	
}

void loop()
{
	
}

int main(void)
{
	setup();
	sei();
    while(1)
    {
        //TODO:: Please write your application code 
		loop();
    }
}