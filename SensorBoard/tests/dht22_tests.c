/*
 * dht22_tests.cpp
 *
 * Created: 2013-12-28 13:40:22
 *  Author: mikael
 */ 

#if DHT22_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include "../core/board.h"
#include "../drivers/dht22_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

void dht22_tests_setup()
{
	DHT22_begin(&PORTD, 4);	
}

void dht22_tests()
{
#if 0	// Hela den här rutinen och drivern ska skrivas om!
	DHT22_ERROR_t errorCode;
	int16_t temperature, humidity;
	
	sen_enable();
	_delay_us(5);
	
	errorCode = DHT22_readData();
	switch(errorCode)
	{
		case DHT_ERROR_CHECKSUM:
		puts_P(PSTR("check sum error "));
		//break;
		
		case DHT_ERROR_NONE:
		temperature = DHT22_getTemperatureCInt();
		humidity = DHT22_getHumidityInt();
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
	
	sen_disable();
#endif
}

#ifdef __cplusplus
}
#endif

#endif