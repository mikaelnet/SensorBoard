/*
 * SensorBoard.cpp
 *
 * Created: 2013-12-27 22:14:11
 *  Author: mikael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "core/cpu.h"
#include "core/console.h"
#include "drivers/dht22_driver.h"

void setup_console()
{
	console_init();
	puts_P(PSTR("Sensor board v0.1"));
}	

DHT22 dht22(&PORTD, 4);


void setup()
{
#if F_CPU == 32000000UL
	cpu_set_32_MHz();
#endif
	setup_console();

	DHT22_ERROR_t errorCode;
	int16_t temperature, humidity;
	errorCode = dht22.readData();
	switch(errorCode)
	{
		case DHT_ERROR_CHECKSUM:
			puts_P(PSTR("check sum error "));
			//break;
			
		case DHT_ERROR_NONE:
			temperature = dht22.getTemperatureCInt();
			humidity = dht22.getHumidityInt();
			printf_P(PSTR("Temperature: %d.%01d %cC\n"), temperature/10, temperature % 10, 0xB0);
			printf_P(PSTR("Humidity: %d.%01d %% RH\n "), humidity/10, humidity%10);
			break;
			
		case DHT_BUS_HUNG:
			puts_P(PSTR("BUS Hung"));
			break;
			
		case DHT_ERROR_NOT_PRESENT:
			puts_P(PSTR("Not Present"));
			break;
			
		case DHT_ERROR_ACK_TOO_LONG:
			puts_P(PSTR("ACK time out"));
			break;
			
		case DHT_ERROR_SYNC_TIMEOUT:
			puts_P(PSTR("Sync Timeout"));
			break;
			
		case DHT_ERROR_DATA_TIMEOUT:
			puts_P(PSTR("Data Timeout"));
			break;
			
		case DHT_ERROR_TOOQUICK:
			puts_P(PSTR("Polled to quick"));
			break;
	}
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