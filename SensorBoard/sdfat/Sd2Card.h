/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */

#ifndef SPICARD_H
#define SPICARD_H

#include <stddef.h>

/**
 * \file
 * \brief Sd2Card class for V2 SD/SDHC cards
 */

#include "SdFatConfig.h"
#include "SdInfo.h"

//------------------------------------------------------------------------------
/** init timeout ms */
#define SD_INIT_TIMEOUT    2000
/** erase timeout ms */
#define SD_ERASE_TIMEOUT   10000
/** read timeout ms */
#define SD_READ_TIMEOUT    300
/** write time out ms */
#define SD_WRITE_TIMEOUT   600

//------------------------------------------------------------------------------
// SD card errors
/** timeout error for command CMD0 (initialize card in SPI mode) */
#define SD_CARD_ERROR_CMD0     0x01
/** CMD8 was not accepted - not a valid SD card*/
#define SD_CARD_ERROR_CMD8     0x02
/** card returned an error response for CMD12 (stop multi block read) */
#define SD_CARD_ERROR_CMD12     0x03
/** card returned an error response for CMD17 (read block) */
#define SD_CARD_ERROR_CMD17     0x04
/** card returned an error response for CMD18 (read multiple block) */
#define SD_CARD_ERROR_CMD18     0x05
/** card returned an error response for CMD24 (write block) */
#define SD_CARD_ERROR_CMD24     0x06
/**  WRITE_MULTIPLE_BLOCKS command failed */
#define SD_CARD_ERROR_CMD25     0x07
/** card returned an error response for CMD58 (read OCR) */
#define SD_CARD_ERROR_CMD58     0x08
/** SET_WR_BLK_ERASE_COUNT failed */
#define SD_CARD_ERROR_ACMD23     0x09
/** ACMD41 initialization process timeout */
#define SD_CARD_ERROR_ACMD41     0x0A
/** card returned a bad CSR version field */
#define SD_CARD_ERROR_BAD_CSD     0x0B
/** erase block group command failed */
#define SD_CARD_ERROR_ERASE     0x0C
/** card not capable of single block erase */
#define SD_CARD_ERROR_ERASE_SINGLE_BLOCK     0x0D
/** Erase sequence timed out */
#define SD_CARD_ERROR_ERASE_TIMEOUT     0x0E
/** card returned an error token instead of read data */
#define SD_CARD_ERROR_READ     0x0F
/** read CID or CSD failed */
#define SD_CARD_ERROR_READ_REG     0x10
/** timeout while waiting for start of read data */
#define SD_CARD_ERROR_READ_TIMEOUT     0x11
/** card did not accept STOP_TRAN_TOKEN */
#define SD_CARD_ERROR_STOP_TRAN     0x12
/** card returned an error token as a response to a write operation */
#define SD_CARD_ERROR_WRITE     0x13
/** attempt to write protected block zero */
#define SD_CARD_ERROR_WRITE_BLOCK_ZERO     0x14  // REMOVE - not used
/** card did not go ready for a multiple block write */
#define SD_CARD_ERROR_WRITE_MULTIPLE     0x15
/** card returned an error to a CMD13 status check after a write */
#define SD_CARD_ERROR_WRITE_PROGRAMMING     0x16
/** timeout occurred during write programming */
#define SD_CARD_ERROR_WRITE_TIMEOUT     0x17
/** incorrect rate selected */
#define SD_CARD_ERROR_SCK_RATE     0x18
/** init() not called */
#define SD_CARD_ERROR_INIT_NOT_CALLED     0x19
/** card returned an error for CMD59 (CRC_ON_OFF) */
#define SD_CARD_ERROR_CMD59     0x1A
/** invalid read CRC */
#define SD_CARD_ERROR_READ_CRC     0x1B
/** SPI DMA error */
#define SD_CARD_ERROR_SPI_DMA     0x1C

//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD1   1
/** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SD2   2
/** High Capacity SD card */
#define SD_CARD_TYPE_SDHC  3

//------------------------------------------------------------------------------
/**
 * \class Sd2Card
 * \brief Raw access to SD and SDHC flash memory cards.
 */
typedef struct Sd2Card_struct {
    uint8_t m_errorCode;
    uint8_t m_status;
    uint8_t m_type;
    uint8_t (*spi_transeiveByte)(uint8_t data);
    void (*spi_chipSelect)(bool selectHigh);
    uint16_t (*cpu_milliseconds)();
} Sd2Card_t;

// Provide SPI transeive and chip select functions
bool Sd2Card_init (Sd2Card_t *sd2card, uint8_t (*spi_transeiveByte)(uint8_t data),
                   void (*spi_chipSelect)(bool select), uint16_t (*cpu_milliseconds)());

uint32_t Sd2Card_cardSize(Sd2Card_t *sd2card);
bool Sd2Card_erase(Sd2Card_t *sd2card, uint32_t firstBlock, uint32_t lastBlock);
bool Sd2Card_eraseSingleBlockEnable(Sd2Card_t *sd2card);

/**
 *  Set SD error code.
 *  \param[in] code value for error code.
 */
void Sd2Card_error(Sd2Card_t *sd2card, uint8_t code);
/**
 * \return error code for last error. See Sd2Card.h for a list of error codes.
 */
int Sd2Card_errorCode(Sd2Card_t *sd2card);
/** \return error data for last error. */
int Sd2Card_errorData(Sd2Card_t *sd2card);

bool Sd2Card_readBlock(Sd2Card_t *sd2card, uint32_t block, uint8_t* dst);
/**
 * Read a card's CID register. The CID contains card identification
 * information such as Manufacturer ID, Product name, Product serial
 * number and Manufacturing date.
 *
 * \param[out] cid pointer to area for returned data.
 *
 * \return true for success or false for failure.
 */
bool Sd2Card_readCID(Sd2Card_t *sd2card, cid_t* cid);
/**
 * Read a card's CSD register. The CSD contains Card-Specific Data that
 * provides information regarding access to the card's contents.
 *
 * \param[out] csd pointer to area for returned data.
 *
 * \return true for success or false for failure.
 */
bool Sd2Card_readCSD(Sd2Card_t *sd2card, csd_t* csd);
bool Sd2Card_readData(Sd2Card_t *sd2card, uint8_t *dst);
bool Sd2Card_readDataBlock(Sd2Card_t *sd2card, uint8_t *dst, size_t count);
bool Sd2Card_readStart(Sd2Card_t *sd2card, uint32_t blockNumber);
bool Sd2Card_readStop(Sd2Card_t *sd2card);
/** Return the card type: SD V1, SD V2 or SDHC
* \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
*/
int Sd2Card_getType(Sd2Card_t *sd2card);
bool Sd2Card_writeBlock(Sd2Card_t *sd2card, uint32_t blockNumber, const uint8_t* src);
bool Sd2Card_writeData(Sd2Card_t *sd2card, const uint8_t* src);
bool Sd2Card_writeDataWithToken(Sd2Card_t *sd2card, uint8_t token, const uint8_t* src);
bool Sd2Card_writeStart(Sd2Card_t *sd2card, uint32_t blockNumber, uint32_t eraseCount);
bool Sd2Card_writeStop(Sd2Card_t *sd2card);

#endif  // SpiCard_h
