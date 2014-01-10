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

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "spi_driver.h"
#include "SD_driver.h"

inline static void SD_CS_assert (SPI_Master_t *spi) {
	SPI_MasterSSLow(spi->port, SPI_SS_bm);
}

inline static void SD_CS_deassert (SPI_Master_t *spi) {
	SPI_MasterSSHigh(spi->port, SPI_SS_bm);
}


//******************************************************************
//Function	: to initialize the SD/SDHC card in SPI mode
//Arguments	: none
//return	: uint8_t; will be 0 if no error,
// 			  otherwise the response byte will be sent
//******************************************************************
uint8_t SD_init(SD_t *sd, SPI_Master_t *spi)
{
	uint8_t i, response, SD_version;
	uint16_t retry=0;

	sd->spi = spi;

	for (i=0; i<10; i++)
		SPI_transmit(spi, 0xff);   //80 clock pulses spent before sending the first command

	SD_CS_assert (spi);
	do {
		response = SD_sendCommand(sd, GO_IDLE_STATE, 0); //send 'reset & go idle' command
		retry++;
		if(retry>0x20) 
			return 1;   //time out, card not detected
	} while(response != 0x01);

	SD_CS_deassert (spi);
	SPI_transmit (spi, 0xff);
	SPI_transmit (spi, 0xff);

	SD_version = 2; //default set to SD compliance with ver2.x; 
					//this may change after checking the next command
	retry = 0;
	do {
		response = SD_sendCommand(sd, SEND_IF_COND,0x000001AA); //Check power supply status, mandatory for SDHC card
		retry ++;
		if (retry > 0xFE) {
			//TX_NEWLINE;
			SD_version = 1;
			sd->cardType = 1;
			break;
		} //time out
	} while(response != 0x01);

	retry = 0;
	do {
		response = SD_sendCommand(sd, APP_CMD, 0); //CMD55, must be sent before sending any ACMD command
		response = SD_sendCommand(sd, SD_SEND_OP_COND, 0x40000000); //ACMD41

		retry++;
		if (retry > 0xFE) {
			//TX_NEWLINE;
			return 2;  //time out, card initialization failed
		} 
	} while(response != 0x00);

	sd->SDHC_flag = false;
	if (SD_version == 2) { 
		retry = 0;
		do {
			response = SD_sendCommand(sd, READ_OCR, 0);
			retry++;
			if(retry > 0xFE) {
				//TX_NEWLINE;
				sd->cardType = 0;
				break;
			} //time out
		} while (response != 0x00);

		if (sd->SDHC_flag) 
			sd->cardType = 2;
		else 
			sd->cardType = 3;
	}

	//SD_sendCommand(CRC_ON_OFF, OFF); //disable CRC; deafault - CRC disabled in SPI mode
	//SD_sendCommand(SET_BLOCK_LEN, 512); //set block size to 512; default size is 512
	return 0; //successful return
}

//******************************************************************
//Function	: to send a command to SD card
//Arguments	: uint8_t (8-bit command value)
// 			  & uint32_t (32-bit command argument)
//return	: uint8_t; response byte
//******************************************************************
uint8_t SD_sendCommand(SD_t *sd, uint8_t cmd, uint32_t arg)
{
	uint8_t response, retry=0, status;
	SPI_Master_t *spi = sd->spi;

	//SD card accepts byte address while SDHC accepts block address in multiples of 512
	//so, if it's SD card we need to convert block address into corresponding byte address by 
	//multiplying it with 512. which is equivalent to shifting it left 9 times
	//following 'if' loop does that

	if (!sd->SDHC_flag) {
		if(cmd == READ_SINGLE_BLOCK     ||
		   cmd == READ_MULTIPLE_BLOCKS  ||
		   cmd == WRITE_SINGLE_BLOCK    ||
		   cmd == WRITE_MULTIPLE_BLOCKS ||
		   cmd == ERASE_BLOCK_START_ADDR|| 
		   cmd == ERASE_BLOCK_END_ADDR) {
			arg = arg << 9;
		}	   
	}

	SD_CS_assert (spi);

	SPI_transmit(spi, cmd | 0x40); //send command, first two bits always '01'
	SPI_transmit(spi, arg>>24);
	SPI_transmit(spi, arg>>16);
	SPI_transmit(spi, arg>>8);
	SPI_transmit(spi, arg);

	if(cmd == SEND_IF_COND)	 //it is compulsory to send correct CRC for CMD8 (CRC=0x87) & CMD0 (CRC=0x95)
		SPI_transmit(spi, 0x87);    //for remaining commands, CRC is ignored in SPI mode
	else 
		SPI_transmit(spi, 0x95); 

	while((response = SPI_receive(spi)) == 0xff) //wait response
		if(retry++ > 0xfe) 
			break; //time out error

	if(response == 0x00 && cmd == 58) { //checking response of CMD58
		status = SPI_receive(spi) & 0x40;     //first byte of the OCR register (bit 31:24)
		if(status == 0x40)
			sd->SDHC_flag = true;  //we need it to verify SDHC card
		else 
			sd->SDHC_flag = false;

		SPI_receive(spi); //remaining 3 bytes of the OCR register are ignored here
		SPI_receive(spi); //one can use these bytes to check power supply limits of SD
		SPI_receive(spi); 
	}

	SPI_receive(spi); //extra 8 CLK
	SD_CS_deassert (spi);

	return response; //return state
}

//*****************************************************************
//Function	: to erase specified no. of blocks of SD card
//Arguments	: none
//return	: uint8_t; will be 0 if no error,
// 			  otherwise the response byte will be sent
//*****************************************************************
uint8_t SD_erase (SD_t *sd, uint32_t startBlock, uint32_t totalBlocks)
{
	uint8_t response;

	response = SD_sendCommand(sd, ERASE_BLOCK_START_ADDR, startBlock); //send starting block address
	if(response != 0x00) //check for SD status: 0x00 - OK (No flags set)
		return response;

	response = SD_sendCommand(sd, ERASE_BLOCK_END_ADDR,(startBlock + totalBlocks - 1)); //send end block address
	if(response != 0x00)
		return response;

	response = SD_sendCommand(sd, ERASE_SELECTED_BLOCKS, 0); //erase all selected blocks
	if(response != 0x00)
		return response;

	return 0; //normal return
}

//******************************************************************
//Function	: to read a single block from SD card
//Arguments	: none
//return	: uint8_t; will be 0 if no error,
// 			  otherwise the response byte will be sent
//******************************************************************
uint8_t SD_readSingleBlock(SD_t *sd, uint32_t startBlock)
{
	uint8_t response;
	uint16_t i, retry=0;
	SPI_Master_t *spi = sd->spi;

	response = SD_sendCommand(sd, READ_SINGLE_BLOCK, startBlock); //read a Block command
 
	if(response != 0x00)
		return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_assert (spi);
	retry = 0;
	while (SPI_receive(spi) != 0xfe) { //wait for start block token 0xfe (0x11111110)
		if(retry++ > 0xfffe) {
			//return if time-out
			SD_CS_deassert (spi); 
			return 1;
		} 
	}

	uint8_t *buf = sd->buffer;
	for(i=0; i<512; i++) //read 512 bytes
		*buf ++ = SPI_receive(spi);

	SPI_receive(spi); //receive incoming CRC (16-bit), CRC is ignored here
	SPI_receive(spi);

	SPI_receive(spi); //extra 8 clock pulses
	SD_CS_deassert (spi);

	return 0;
}

//******************************************************************
//Function	: to write to a single block of SD card
//Arguments	: none
//return	: uint8_t; will be 0 if no error,
// 			  otherwise the response byte will be sent
//******************************************************************
uint8_t SD_writeSingleBlock(SD_t *sd, uint32_t startBlock)
{
	uint8_t response;
	uint16_t i, retry=0;
	SPI_Master_t *spi = sd->spi;
	
	response = SD_sendCommand(sd, WRITE_SINGLE_BLOCK, startBlock); //write a Block command
  
	if (response != 0x00) 
		return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_assert (spi);
	SPI_transmit(spi, 0xfe);     //Send start block token 0xfe (0x11111110)

	uint8_t *buf = sd->buffer;
	for(i=0; i<512; i++)    //send 512 bytes data
		SPI_transmit(spi, *buf++);

	SPI_transmit(spi, 0xff);     //transmit dummy CRC (16-bit), CRC is ignored here
	SPI_transmit(spi, 0xff);

	response = SPI_receive(spi);
	if( (response & 0x1f) != 0x05) { //response= 0xXXX0AAA1 ; AAA='010' - data accepted
		SD_CS_deassert (spi);				 //AAA='101'-data rejected due to CRC error
		return response;             //AAA='110'-data rejected due to write error
	}

	while (!SPI_receive(spi)) { //wait for SD card to complete writing and get idle
		if (retry++ > 0xfffe) {
			SD_CS_deassert (spi); 
			return 1;
		}
	}
	SD_CS_deassert (spi);
	SPI_transmit(spi, 0xff);   //just spend 8 clock cycle delay before reasserting the CS line
	SD_CS_assert (spi);         //re-asserting the CS line to verify if card is still busy

	while (!SPI_receive(spi)) { //wait for SD card to complete writing and get idle
		if(retry++ > 0xfffe) {
			SD_CS_deassert (spi); 
			return 1;
		}
	}
	SD_CS_deassert (spi);

	return 0;
}


#ifndef FAT_TESTING_ONLY

//***************************************************************************
//Function	: to read multiple blocks from SD card & send every block to UART
//Arguments	: none
//return	: uint8_t; will be 0 if no error,
// 			  otherwise the response byte will be sent
//****************************************************************************
uint8_t SD_readMultipleBlock (SD_t *sd, uint32_t startBlock, uint32_t totalBlocks)
{
	uint8_t response;
	uint16_t i, retry=0;
	SPI_Master_t *spi = sd->spi;

	response = SD_sendCommand(sd, READ_MULTIPLE_BLOCKS, startBlock); //write a Block command
  
	if (response != 0x00) 
		return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_assert (spi);

	retry = 0;
	while (totalBlocks) {
		retry = 0;
		while (SPI_receive(spi) != 0xfe) { //wait for start block token 0xfe (0x11111110)
			if (retry++ > 0xfffe) {
				SD_CS_deassert (spi); 
				return 1;	 //return if time-out
			}
		}
		
		for(i=0; i<512; i++) //read 512 bytes
			sd->buffer[i] = SPI_receive(spi);

		SPI_receive(spi); //receive incoming CRC (16-bit), CRC is ignored here
		SPI_receive(spi);

		SPI_receive(spi); //extra 8 cycles
		//TX_NEWLINE;
		//transmitString_F(PSTR(" --------- "));
		//TX_NEWLINE;

		for(i=0; i<512; i++) //send the block to UART
		{
			if (sd->buffer[i] == '~') 
				break;
			//transmitByte ( buffer[i] );
		}

		//TX_NEWLINE;
		//transmitString_F(PSTR(" --------- "));
		//TX_NEWLINE;
		totalBlocks--;
	}

	SD_sendCommand(sd, STOP_TRANSMISSION, 0); //command to stop transmission
	SD_CS_deassert (spi);
	SPI_receive(spi); //extra 8 clock pulses

	return 0;
}

//***************************************************************************
//Function: to receive data from UART and write to multiple blocks of SD card
//Arguments: none
//return: uint8_t; will be 0 if no error,
// otherwise the response byte will be sent
//****************************************************************************
uint8_t SD_writeMultipleBlock(SD_t *sd, uint32_t startBlock, uint32_t totalBlocks)
{
	uint8_t response, data;
	uint16_t i, retry=0;
	uint32_t blockCounter=0, size;
	SPI_Master_t *spi = sd->spi;

	response = SD_sendCommand (sd, WRITE_MULTIPLE_BLOCKS, startBlock); //write a Block command

	if (response != 0x00) 
		return response; //check for SD status: 0x00 - OK (No flags set)

	SD_CS_assert (spi);

	//TX_NEWLINE;
	//transmitString_F(PSTR(" Enter text (End with ~): "));
	//TX_NEWLINE;

	while( blockCounter < totalBlocks ) {
		i=0;
		do {
			//data = receiveByte();
			if(data == 0x08) {	//'Back Space' key pressed
				if(i != 0)	{ 
					//transmitByte(data);
					//transmitByte(' '); 
					//transmitByte(data); 
					i--; 
					size--;
				} 
				continue;     
			}
			
			//transmitByte(data);
			sd->buffer[i++] = data;
			if(data == 0x0d) {
				//transmitByte(0x0a);
				sd->buffer[i++] = 0x0a;
			}
			if(i == 512) 
				break;
		} while (data != '~');

		//TX_NEWLINE;
		//transmitString_F(PSTR(" ---- "));
		//TX_NEWLINE;

		SPI_transmit (spi, 0xfc); //Send start block token 0xfc (0x11111100)

		for(i=0; i<512; i++) //send 512 bytes data
			SPI_transmit(spi, sd->buffer[i]);

		SPI_transmit(spi, 0xff); //transmit dummy CRC (16-bit), CRC is ignored here
		SPI_transmit(spi, 0xff);

		response = SPI_receive(spi);
		if( (response & 0x1f) != 0x05) { //response= 0xXXX0AAA1 ; AAA='010' - data accepted
			SD_CS_deassert (spi);              //AAA='101'-data rejected due to CRC error 
			return response;			 //AAA='110'-data rejected due to write error
		}

		while (!SPI_receive(spi)) { //wait for SD card to complete writing and get idle
			if (retry++ > 0xfffe) {
				SD_CS_deassert (spi); 
				return 1;
			}
		}

		SPI_receive(spi); //extra 8 bits
		blockCounter++;
	}

	SPI_transmit(spi, 0xfd); //send 'stop transmission token'

	retry = 0;
	while (!SPI_receive(spi)) { //wait for SD card to complete writing and get idle
		if (retry++ > 0xfffe) {
			SD_CS_deassert (spi); 
			return 1;
		}
	}
	
	SD_CS_deassert (spi);
	SPI_transmit(spi, 0xff); //just spend 8 clock cycle delay before reasserting the CS signal
	SD_CS_assert (spi); //re assertion of the CS signal is required to verify if card is still busy

	while (!SPI_receive(spi)) { //wait for SD card to complete writing and get idle
		if (retry++ > 0xfffe) {
			SD_CS_deassert (spi); 
			return 1;
		} 
	}
	SD_CS_deassert (spi);

	return 0;
}
//*********************************************

#endif
