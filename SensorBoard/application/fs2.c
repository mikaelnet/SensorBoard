/*
 * fs2.c
 *
 * Created: 2014-05-20 20:27:01
 *  Author: mikael
 */

#include "fs2.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "terminal.h"
#include "../drivers/spi_driver.h"
#include "../core/process.h"
#include "../core/board.h"
#include "../core/cpu.h"

#include "../sdfat/Sd2Card.h"
#include "../sdfat/SdVolume.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "FS";

static Process_t filesystem_process;
static CPU_SleepMethod_t sleep_methods;
static void (*stateMethod)();

static SPI_Master_t spi;
static Sd2Card_t sdCard;



static void memory_dump (uint16_t baseAddress, uint8_t *data, uint16_t length)
{
    for (uint16_t i=0 ; i < length ; i += 16) {
        printf_P(PSTR("%04X  "), baseAddress);
        for (uint8_t j=0 ; j < 16 ; j ++) {
            if (i+j < length)
                printf_P(PSTR("%02X "), data[j]);
            else
                fputs_P(PSTR("   "), stdout);

            if (j == 8)
                fputc(' ', stdout);
        }
        fputs_P(PSTR("  "), stdout);
        for (uint8_t j=0 ; j < 16 ; j ++) {
            if (i+j < length) {
                char ch = data[j];
                if (ch < 32)
                    fputc('.', stdout);
                else
                    fputc(ch, stdout);
            }
            else
                fputc(' ', stdout);

            if (j == 8)
                fputc(' ', stdout);
        }
        fputc('\n', stdout);
        baseAddress += 16;
        data += 16;
    }
}


static void filesystem_test()
{
    puts_P(PSTR("SD Card tests"));
    printf_P(PSTR("Card type: %d\n"), Sd2Card_getType(&sdCard));

    cid_t cid;
    Sd2Card_readCID(&sdCard, &cid);
    puts_P(PSTR("CID dump"));
    memory_dump(0x00, (uint8_t*)&cid, sizeof(cid));
    printf_P(PSTR("Manufacturer ID: %d\n"), cid.mid);
    printf_P(PSTR("Serial: %lu\n"), cid.psn);

    csd_t csd;
    Sd2Card_readCSD(&sdCard, &csd);
    memory_dump(0x00, (uint8_t*)&csd, sizeof(csd));
}

static bool parse_command (const char *args)
{
    if (strcasecmp_P(args, PSTR("TEST")) == 0) {
        filesystem_test();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("File system on microSD card"));
}

static void print_help ()
{
    puts_P(PSTR("TEST   Run file system tests"));
}

static void filesystem_loop ()
{
    // check time if we should calculate temperature. Maybe this should be an event only?
}

static bool can_sleep() {
    return stateMethod == NULL;
}

static void before_sleep() {
    stateMethod = NULL;
    thsen_disable();
}

static void after_wakeup() {

}

static uint8_t spi_tranceiveByte (uint8_t data)
{
    return SPI_MasterTransceiveByte(&spi, data);
}

static void spi_chipSelect (bool selectHigh)
{
    if (selectHigh)
        SPI_MasterSSHigh(&PORTC, SPI_SS_bm);
    else
        SPI_MasterSSLow(&PORTC, SPI_SS_bm);
}

void fs2_init ()
{
    PORTC.DIRSET = SPI_SS_bm;
    PORTC.PIN4CTRL = PORT_OPC_WIREDANDPULL_gc;
    PORTC.OUTSET = SPI_SS_bm;

    SPI_MasterInit(&spi, &SPIC, &PORTC, false, SPI_MODE_0_gc, false, SPI_PRESCALER_DIV16_gc);
    puts_P(PSTR("Init SD"));
    if (!Sd2Card_init(&sdCard, &spi_tranceiveByte, &spi_chipSelect, &cpu_millisecond)) {
        printf_P(PSTR("Failed to init SD card: %d\n"), sdCard.m_errorCode);
    }

    //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
    //clock rate: 125Khz
    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    cpu_register_sleep_methods(&sleep_methods, &can_sleep, &before_sleep, &after_wakeup);
    process_register(&filesystem_process, &filesystem_loop, NULL);
}
