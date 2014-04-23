/*
 * thermometer.c
 *
 * Created: 2014-01-13 20:32:38
 *  Author: mikael
 */

#include "thermometer.h"
#include "../drivers/ds1820_driver.h"
#include "../drivers/onewire_driver.h"
#include "../core/process.h"
#include "../core/board.h"
#include "../core/cpu.h"
#include "transmitter.h"
#include "terminal.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define MAX_DEVICES     4

static Process_t thermometer_process;
static CPU_SleepMethod_t sleep_methods;
static OneWire_t oneWire;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "TEMP";

/*typedef enum thermometer_state_enum {
    Idle,
    PowerUp,
    Reading,
} thermometer_state_t;
static thermometer_state_t _state;*/

static void (*stateMethod)();
static uint16_t _startTime;

typedef struct ThermometerDevice_struct {
    uint8_t address[8];
    struct ThermometerDevice_struct *next;
} ThermometerDevice_t;
static ThermometerDevice_t deviceArray[MAX_DEVICES];
static uint8_t numDevices = 0;
static uint8_t currentDevice = 0;

static void thermometer_on_begin_read();

static void thermometer_on_read_complete() {
    if (cpu_millisecond() - _startTime < 750)
        return;

    uint16_t raw = DS1820_ReadTemperature (&oneWire, deviceArray[currentDevice].address);
    char temperature[10];
    snprintf_P(temperature, sizeof(temperature), PSTR(" %4d.%01d%cC"), raw >> 4, (raw << 12) / 6553, 0xB0);
    printf_P(PSTR("\nTemperature: %s\n"), temperature);

    currentDevice ++;
    if (currentDevice < numDevices) {
        currentDevice ++;
        stateMethod = &thermometer_on_begin_read;
    }
    else
        stateMethod = NULL;
}

static void thermometer_on_begin_read() {
    printf_P(PSTR("Reading device %d..."), currentDevice+1);
    
    DS1820_StartConvertion (&oneWire, deviceArray[currentDevice].address);
    
    _startTime = cpu_millisecond();
    stateMethod = &thermometer_on_read_complete;
}

static void thermometer_on_powerup() {
    if (cpu_millisecond() - _startTime < 500)
    return;
    currentDevice = 0;
    stateMethod = &thermometer_on_begin_read;
}

static void thermometer_get_temp()
{
    if (numDevices == 0) {
        puts_P(PSTR("No devices. aborting."));
        return;
    }
    
    puts_P(PSTR("Reading temperature"));
    if (stateMethod != NULL) {
        puts_P(PSTR("Incorrect state. aborting."));
        return;
    }

    thsen_enable();
    _startTime = cpu_millisecond();
    stateMethod = &thermometer_on_powerup;
}

static void thermometer_scan_devices()
{
    puts_P(PSTR("Scanning for DS18S20 devices"));
    thsen_enable();
    _delay_ms(500);
    
    //currentDevice = deviceArray[0];
    numDevices = 0;
    ThermometerDevice_t *dev = &deviceArray[0];
    for (bool found = DS1820_FindFirst(&oneWire, dev->address) ; 
        found && numDevices < MAX_DEVICES ; 
        found = DS1820_FindNext(&oneWire, dev->address)) {
        
        dev->next = NULL;
        if (numDevices > 0)
            deviceArray[numDevices-1].next = dev;
        
        printf_P(PSTR("Device %d at "));
        for (uint8_t i = 0 ; i < 8 ; i ++)
            printf_P(PSTR(" %02X"), dev->address[i]);
        printf_P(PSTR("\n"));
        
        numDevices ++;
        dev = &deviceArray[numDevices];
    }
}

static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        thermometer_get_temp();
        return true;
    }
    if (strcasecmp_P(args, PSTR("SCAN")) == 0) {
        thermometer_scan_devices();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get temperature from DS18B20 temp sensor"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
    puts_P(PSTR("SCAN  Scan for devices"));
}

static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == MINUTE) {
        // Here we may read temperature...
        thermometer_get_temp();
    }
}

static void loop () {
    if (stateMethod != NULL)
        stateMethod();
}

static bool can_sleep() {
    return stateMethod == NULL;
}

static void before_sleep() {
    stateMethod = NULL;
    thsen_disable();
}

void thermometer_init()
{
    OneWire_init(&oneWire, &PORTD, 5);
    thermometer_scan_devices();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    cpu_register_sleep_methods(&sleep_methods, &can_sleep, &before_sleep, NULL);
    process_register(&thermometer_process, &loop, &event_handler);
}


/*
static void thermometer_get_temp2()
{
    puts_P(PSTR("Reading temperature using method 2"));
    thsen_enable();
    _delay_ms(500);

    puts_P(PSTR("Searching bus..."));

    uint8_t addr[8];
    while (DS1820_FindNext(&oneWire, addr)) {
        printf_P(PSTR("ADDR:"));
        for (uint8_t i = 0 ; i < 8 ; i ++)
            printf_P(PSTR(" %02X"), addr[i]);
        printf_P(PSTR("\n"));
    }

    puts_P(PSTR("Reading specific address..."));
    DS1820_StartConvertion (&oneWire, addr);
    _delay_ms(750);     // maybe 750ms is enough, maybe not
    uint16_t raw = DS1820_ReadTemperature (&oneWire, addr);
    char temperature[10];
    snprintf_P(temperature, sizeof(temperature), PSTR(" %4d.%01d%cC"), raw >> 4, (raw << 12) / 6553, 0xB0);
    printf_P(PSTR("\nTemperature: %s\n"), temperature);

    thsen_disable();
}

static void thermometer_get_temp()
{
    puts_P(PSTR("Reading thermometers"));
    thsen_enable();
    _delay_ms(500);

    uint8_t addr[8], data[12], type_s;
    //OneWire_reset_search(&oneWire);
    while (OneWire_search(&oneWire, addr)) {
        switch(addr[0]) {
            case 0x10:
                puts_P(PSTR("  Chip = DS18S20"));  // or old DS1820
                type_s = 1;
                break;
            case 0x28:
                puts_P(PSTR("  Chip = DS18B20"));
                type_s = 0;
                break;
            case 0x22:
                puts_P(PSTR("  Chip = DS1822"));
                type_s = 0;
                break;
            default:
                puts_P(PSTR("Device is not a DS18x20 family device."));
                return;
        }

        printf_P(PSTR("ADDR:"));
        for (uint8_t i = 0 ; i < 8 ; i ++)
            printf_P(PSTR(" %02X"), addr[i]);
        printf_P(PSTR("\n"));

        OneWire_reset(&oneWire);
        OneWire_select(&oneWire, addr);
        OneWire_write(&oneWire, 0x44, false);         // start conversion, with parasite power on at the end

        _delay_ms(750);     // maybe 750ms is enough, maybe not
        OneWire_reset(&oneWire);
        OneWire_select(&oneWire, addr);
        OneWire_write(&oneWire, 0xBE, false);         // Read scratch pad

        OneWire_read_bytes(&oneWire, data, 9);
        OneWire_reset(&oneWire);

        printf_P(PSTR("  Data "));
        for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
            printf_P(PSTR(" %02X"), data[i]);
        }


        uint16_t raw = (data[1] << 8) | data[0];
        if (type_s) {
            raw = raw << 3; // 9 bit resolution default
            if (data[7] == 0x10) {
                // count remain gives full 12 bit resolution
                raw = (raw & 0xFFF0) + 12 - data[6];
            }
        }
        else {
            uint8_t cfg = (data[4] & 0x60);
            if (cfg == 0x00)
                raw = raw << 3;  // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20)
                raw = raw << 2; // 10 bit res, 187.5 ms
            else if (cfg == 0x40)
                raw = raw << 1; // 11 bit res, 375 ms
            // default is 12 bit resolution, 750 ms conversion time
        }

        char temperature[10];
        snprintf_P(temperature, sizeof(temperature), PSTR(" %4d.%01d%cC"), raw >> 4, (raw << 12) / 6553, 0xB0);
        printf_P(PSTR("\nTemperature: %s\n"), temperature);
        temperature[0] = 0x53;  // ID
        transmitter_debug(temperature, 9);
    }

    thsen_disable();
}
*/
