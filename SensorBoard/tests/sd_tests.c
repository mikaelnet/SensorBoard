/*
 * sd_tests.c
 *
 * Created: 2014-01-10 15:59:42
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sd_tests.h"

#include "../drivers/spi_driver.h"
#include "../drivers/SD_driver.h"
#include "../drivers/FAT32_driver.h"

SPI_Master_t spi;
SD_t sdCard;
FAT32_FS_t fat32_FS;

void sd_tests_setup()
{
    //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
    //clock rate: 125Khz
    SPI_MasterInit(&spi, &SPIC, &PORTC, false, SPI_MODE_0_gc, false, SPI_PRESCALER_DIV128_gc);
    SD_init(&sdCard, &spi);
    FAT32_init(&fat32_FS, &sdCard, stdout, stdin);
}

void sd_tests()
{
    
}