//**************************************************************
// ****** FUNCTIONS FOR SD RAW DATA TRANSFER *******
//**************************************************************
//Controller: ATmega32 (Clock: 8 Mhz-internal)
//Compiler	: AVR-GCC (winAVR with AVRStudio)
//Project V.: Version - 2.4.1
//Author	: CC Dharmani, Chennai (India)
//			  www.dharmanitech.com
//Date		: 24 Apr 2011
//**************************************************************

//Link to the Post: http://www.dharmanitech.com/2009/01/sd-card-interfacing-with-atmega8-fat32.html

#ifndef _SD_DRIVER_H_
#define _SD_DRIVER_H_

#include "spi_driver.h"
#include <stdbool.h>

//Use following macro if you don't want to activate the multiple block access functions
//those functions are not required for FAT32

#define FAT_TESTING_ONLY

//SD commands, many of these are not used here
#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_IF_COND			 8
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define SD_SEND_OP_COND			 41   //ACMD
#define APP_CMD					 55
#define READ_OCR				 58
#define CRC_ON_OFF               59

typedef struct SD_struct {
	SPI_Master_t *spi;
	uint8_t cardType; 		// volatile
	bool SDHC_flag;			// volatile
	uint8_t buffer[512];	// volatile
} SD_t;

uint8_t SD_init(SD_t *sd, SPI_Master_t *spi);
uint8_t SD_sendCommand(SD_t *sd, uint8_t cmd, uint32_t arg);
uint8_t SD_erase (SD_t *sd, uint32_t startBlock, uint32_t totalBlocks);
uint8_t SD_readSingleBlock(SD_t *sd, uint32_t startBlock);
uint8_t SD_writeSingleBlock(SD_t *sd, uint32_t startBlock);
#ifndef FAT_TESTING_ONLY
uint8_t SD_readMultipleBlock (SD_t *sd, uint32_t startBlock, uint32_t totalBlocks);
uint8_t SD_writeMultipleBlock(SD_t *sd, uint32_t startBlock, uint32_t totalBlocks);
#endif


//#warning Re-factor below:
extern uint8_t SPI_transmit(SPI_Master_t *spi, uint8_t data);
extern uint8_t SPI_receive(SPI_Master_t *spi);

#endif
