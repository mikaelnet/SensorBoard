/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */

#include "Sd2Card.h"

// debug trace macro
#define SD_TRACE(m, b)
// #define SD_TRACE(m, b)  fputs(m, stdout);puts(b);


//static bool Sd2Card_readDataBlock(Sd2Card_t *sd2card, uint8_t* dst, size_t count);
static bool Sd2Card_readRegister(Sd2Card_t *sd2card, uint8_t cmd, void* buf);
static void Sd2Card_chipSelectHigh(Sd2Card_t *sd2card);
static void Sd2Card_chipSelectLow(Sd2Card_t *sd2card);
static bool Sd2Card_waitNotBusy(Sd2Card_t *sd2card, uint16_t timeoutMillis);

static void Sd2Card_setType (Sd2Card_t *sd2card, uint8_t value) {
    sd2card->m_type = value;
}

//==============================================================================
// Sd2Card member functions
//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
static uint8_t Sd2Card_cardCommand(Sd2Card_t *sd2card, uint8_t cmd, uint32_t arg) {
    // select card
    Sd2Card_chipSelectLow(sd2card);

    // wait if busy
    Sd2Card_waitNotBusy(sd2card, SD_WRITE_TIMEOUT);

    uint8_t *pa = (uint8_t *)(&arg);

    // send command
    sd2card->spi_transeiveByte(cmd | 0x40);

    // send argument
    for (int8_t i = 3; i >= 0; i--)
        sd2card->spi_transeiveByte(pa[i]);

    // send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
    sd2card->spi_transeiveByte (cmd == CMD0 ? 0X95 : 0X87);

    // skip stuff byte for stop read
    if (cmd == CMD12)
        sd2card->spi_transeiveByte(0x00); // receive

    // wait for response
    for (uint8_t i = 0; ((sd2card->m_status = sd2card->spi_transeiveByte(0x00)) & 0X80) && i != 0XFF; i++)
        ;
    return sd2card->m_status;
}

static uint8_t Sd2Card_cardAcmd(Sd2Card_t *sd2card, uint8_t cmd, uint32_t arg) {
    Sd2Card_cardCommand(sd2card, CMD55, 0);
    return Sd2Card_cardCommand(sd2card, cmd, arg);
}


//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t Sd2Card_cardSize(Sd2Card_t *sd2card) {
    csd_t csd;
    if (!Sd2Card_readCSD(sd2card, &csd))
        return 0;

    if (csd.v1.csd_ver == 0) {
        uint8_t read_bl_len = csd.v1.read_bl_len;
        uint16_t c_size = (csd.v1.c_size_high << 10) | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
        uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1) | csd.v1.c_size_mult_low;
        return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
    }
    else if (csd.v2.csd_ver == 1) {
        uint32_t c_size = 0X10000L * csd.v2.c_size_high + 0X100L * (uint32_t)csd.v2.c_size_mid + csd.v2.c_size_low;
        return (c_size + 1) << 10;
    }
    else {
        Sd2Card_error(sd2card, SD_CARD_ERROR_BAD_CSD);
        return 0;
    }
}

//------------------------------------------------------------------------------
static void Sd2Card_chipSelectHigh(Sd2Card_t *sd2card) {
    sd2card->spi_chipSelect(true);
    // insure MISO goes high impedance
    //m_spi.send(0XFF);
}
//------------------------------------------------------------------------------
static void Sd2Card_chipSelectLow(Sd2Card_t *sd2card) {
    sd2card->spi_chipSelect(false);
    //m_spi.init(m_sckDivisor);
    //digitalWrite(m_chipSelectPin, LOW);
}

//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_erase(Sd2Card_t *sd2card, uint32_t firstBlock, uint32_t lastBlock) {
    csd_t csd;

    if (!Sd2Card_readCSD(sd2card, &csd))
        goto fail;

    // check for single block erase
    if (!csd.v1.erase_blk_en) {
        // erase size mask
        uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
        if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
            // error card can't erase specified area
            Sd2Card_error(sd2card, SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
            goto fail;
        }
    }

    if (sd2card->m_type != SD_CARD_TYPE_SDHC) {
        firstBlock <<= 9;
        lastBlock <<= 9;
    }

    if (Sd2Card_cardCommand(sd2card, CMD32, firstBlock) ||
        Sd2Card_cardCommand(sd2card, CMD33, lastBlock) ||
        Sd2Card_cardCommand(sd2card, CMD38, 0)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_ERASE);
        goto fail;
    }

    if (!Sd2Card_waitNotBusy(sd2card, SD_ERASE_TIMEOUT)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_ERASE_TIMEOUT);
        goto fail;
    }
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
bool Sd2Card_eraseSingleBlockEnable(Sd2Card_t *sd2card) {
    csd_t csd;
    return Sd2Card_readCSD(sd2card, &csd) ? csd.v1.erase_blk_en : false;
}
//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \param[in] chipSelectPin SD chip select pin number.
 * \param[in] sckDivisor SPI SCK clock rate divisor.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
bool Sd2Card_init(Sd2Card_t *sd2card, uint8_t (*spi_transeiveByte)(uint8_t data),
        void (*spi_chipSelect)(bool select), uint16_t (*cpu_milliseconds)()) {
    sd2card->spi_transeiveByte = spi_transeiveByte;
    sd2card->spi_chipSelect = spi_chipSelect;
    sd2card->cpu_milliseconds = cpu_milliseconds;
    sd2card->m_errorCode = 0;
    sd2card->m_type = 0;
    // 16-bit init start time allows over a minute
    uint16_t t0 = sd2card->cpu_milliseconds();
    uint32_t arg;

    Sd2Card_chipSelectHigh(sd2card);

    // must supply min of 74 clock cycles with CS high.
    for (uint8_t i = 0; i < 10; i++)
        sd2card->spi_transeiveByte(0xFF);

    // command to go idle in SPI mode
    while (Sd2Card_cardCommand(sd2card, CMD0, 0) != R1_IDLE_STATE) {
        if ((sd2card->cpu_milliseconds() - t0) > SD_INIT_TIMEOUT) {
            Sd2Card_error(sd2card, SD_CARD_ERROR_CMD0);
            goto fail;
        }
    }
    // check SD version
    while (1) {
        if (Sd2Card_cardCommand(sd2card, CMD8, 0x1AA) == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE)) {
            Sd2Card_setType(sd2card, SD_CARD_TYPE_SD1);
            break;
        }
        for (uint8_t i = 0; i < 4; i++)
            sd2card->m_status = sd2card->spi_transeiveByte(0xFF);
        if (sd2card->m_status == 0xAA) {
            Sd2Card_setType(sd2card, SD_CARD_TYPE_SD2);
            break;
        }
        if ((sd2card->cpu_milliseconds() - t0) > SD_INIT_TIMEOUT) {
            Sd2Card_error(sd2card, SD_CARD_ERROR_CMD8);
            goto fail;
        }
    }

    // initialize card and send host supports SDHC if SD2
    arg = Sd2Card_getType(sd2card) == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

    while (Sd2Card_cardAcmd(sd2card, ACMD41, arg) != R1_READY_STATE) {
        // check for timeout
        if ((sd2card->cpu_milliseconds() - t0) > SD_INIT_TIMEOUT) {
            Sd2Card_error(sd2card, SD_CARD_ERROR_ACMD41);
            goto fail;
        }
    }
    // if SD2 read OCR register to check for SDHC card
    if (Sd2Card_getType(sd2card) == SD_CARD_TYPE_SD2) {
        if (Sd2Card_cardCommand(sd2card, CMD58, 0)) {
            Sd2Card_error(sd2card, SD_CARD_ERROR_CMD58);
            goto fail;
        }
        if ((sd2card->spi_transeiveByte(0xFF) & 0XC0) == 0XC0)
            Sd2Card_setType(sd2card, SD_CARD_TYPE_SDHC);
        // Discard rest of ocr - contains allowed voltage range.
        for (uint8_t i = 0; i < 3; i++)
            sd2card->spi_transeiveByte(0xFF);
    }
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card.
 *
 * \param[in] blockNumber Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_readBlock(Sd2Card_t *sd2card, uint32_t blockNumber, uint8_t* dst) {
    SD_TRACE("RB", blockNumber);
    // use address if not SDHC card
    if (Sd2Card_getType(sd2card)!= SD_CARD_TYPE_SDHC)
        blockNumber <<= 9;
    if (Sd2Card_cardCommand(sd2card, CMD17, blockNumber)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_CMD17);
        goto fail;
    }
    return Sd2Card_readDataBlock(sd2card, dst, 512);

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** Read one data block in a multiple block read sequence
 *
 * \param[in] dst Pointer to the location for the data to be read.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_readData(Sd2Card_t *sd2card, uint8_t *dst) {
    Sd2Card_chipSelectLow(sd2card);
    return Sd2Card_readDataBlock(sd2card, dst, 512);
}
//------------------------------------------------------------------------------
bool Sd2Card_readDataBlock(Sd2Card_t *sd2card, uint8_t* dst, size_t count) {
    // wait for start block token
    uint16_t t0 = sd2card->cpu_milliseconds();
    while ((sd2card->m_status = sd2card->spi_transeiveByte(0xFF)) == 0XFF) {
        if ((sd2card->cpu_milliseconds() - t0) > SD_READ_TIMEOUT) {
            Sd2Card_error(sd2card, SD_CARD_ERROR_READ_TIMEOUT);
            goto fail;
        }
    }

    if (sd2card->m_status != DATA_START_BLOCK) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_READ);
        goto fail;
    }
    // transfer data
    for (size_t i=0 ; i < count ; i ++) {
        *dst = sd2card->spi_transeiveByte(0xFF);
        dst ++;
    }
    /*if ((sd2card->m_status = sd2card->spi_transeiveByte(dst, count))) {
    Sd2Card_error(sd2card, SD_CARD_ERROR_SPI_DMA);
    goto fail;
    }*/

    // discard crc
    sd2card->spi_transeiveByte(0xFF);
    sd2card->spi_transeiveByte(0xFF);

    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** read CID or CSR register */
bool Sd2Card_readRegister(Sd2Card_t *sd2card, uint8_t cmd, void* buf) {
    uint8_t* dst = (uint8_t*)buf;
    if (Sd2Card_cardCommand(sd2card, cmd, 0)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_READ_REG);
        goto fail;
    }
    return Sd2Card_readDataBlock(sd2card, dst, 16);

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** Start a read multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 *
 * \note This function is used with readData() and readStop() for optimized
 * multiple block reads.  SPI chipSelect must be low for the entire sequence.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_readStart(Sd2Card_t *sd2card, uint32_t blockNumber) {
    SD_TRACE("RS", blockNumber);
    if (Sd2Card_getType(sd2card)!= SD_CARD_TYPE_SDHC)
        blockNumber <<= 9;
    if (Sd2Card_cardCommand(sd2card, CMD18, blockNumber)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_CMD18);
        goto fail;
    }
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** End a read multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_readStop(Sd2Card_t *sd2card) {
    if (Sd2Card_cardCommand(sd2card, CMD12, 0)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_CMD12);
        goto fail;
    }
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
// wait for card to go not busy
bool Sd2Card_waitNotBusy(Sd2Card_t *sd2card, uint16_t timeoutMillis) {
    uint16_t t0 = sd2card->cpu_milliseconds();
    while (sd2card->spi_transeiveByte(0xFF) != 0XFF) {
        if ((sd2card->cpu_milliseconds() - t0) >= timeoutMillis)
            goto fail;
    }
    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_writeBlock(Sd2Card_t *sd2card, uint32_t blockNumber, const uint8_t* src) {
    SD_TRACE("WB", blockNumber);
    // use address if not SDHC card
    if (Sd2Card_getType(sd2card) != SD_CARD_TYPE_SDHC)
        blockNumber <<= 9;
    if (Sd2Card_cardCommand(sd2card, CMD24, blockNumber)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_CMD24);
        goto fail;
    }
    if (!Sd2Card_writeDataWithToken(sd2card, DATA_START_BLOCK, src))
        goto fail;

#define CHECK_PROGRAMMING 0
#if CHECK_PROGRAMMING
    // wait for flash programming to complete
    if (!waitNotBusy(sd2card, SD_WRITE_TIMEOUT)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_WRITE_TIMEOUT);
        goto fail;
    }
    // response is r2 so get and check two bytes for nonzero
    if (Sd2Card_cardCommand(sd2card, CMD13, 0) || sd2card->spi_transeiveByte(0xFF)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_WRITE_PROGRAMMING);
        goto fail;
    }
#endif  // CHECK_PROGRAMMING

    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_writeData(Sd2Card_t *sd2card, const uint8_t* src) {
    Sd2Card_chipSelectLow(sd2card);
    // wait for previous write to finish
    if (!Sd2Card_waitNotBusy(sd2card, SD_WRITE_TIMEOUT))
        goto fail;
    if (!Sd2Card_writeDataWithToken(sd2card, WRITE_MULTIPLE_TOKEN, src))
        goto fail;
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_error(sd2card, SD_CARD_ERROR_WRITE_MULTIPLE);
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
bool Sd2Card_writeDataWithToken(Sd2Card_t *sd2card, uint8_t token, const uint8_t* src) {
    uint16_t crc = 0XFFFF;

    sd2card->spi_transeiveByte(token);
    for (uint16_t i = 0 ; i < 512 ; i ++)
        sd2card->spi_transeiveByte(src[i]);
    sd2card->spi_transeiveByte(crc >> 8);
    sd2card->spi_transeiveByte(crc & 0XFF);

    sd2card->m_status = sd2card->spi_transeiveByte(0xFF);
    if ((sd2card->m_status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_WRITE);
        goto fail;
    }
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_writeStart(Sd2Card_t *sd2card, uint32_t blockNumber, uint32_t eraseCount) {
    SD_TRACE("WS", blockNumber);
    // send pre-erase count
    if (Sd2Card_cardAcmd(sd2card, ACMD23, eraseCount)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_ACMD23);
        goto fail;
    }
    // use address if not SDHC card
    if (Sd2Card_getType(sd2card) != SD_CARD_TYPE_SDHC)
        blockNumber <<= 9;
    if (Sd2Card_cardCommand(sd2card, CMD25, blockNumber)) {
        Sd2Card_error(sd2card, SD_CARD_ERROR_CMD25);
        goto fail;
    }
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card_writeStop(Sd2Card_t *sd2card) {
    Sd2Card_chipSelectLow(sd2card);
    if (!Sd2Card_waitNotBusy(sd2card, SD_WRITE_TIMEOUT))
        goto fail;
    sd2card->spi_transeiveByte(STOP_TRAN_TOKEN);
    if (!Sd2Card_waitNotBusy(sd2card, SD_WRITE_TIMEOUT))
        goto fail;
    Sd2Card_chipSelectHigh(sd2card);
    return true;

    fail:
    Sd2Card_error(sd2card, SD_CARD_ERROR_STOP_TRAN);
    Sd2Card_chipSelectHigh(sd2card);
    return false;
}

bool Sd2Card_readCID(Sd2Card_t *sd2card, cid_t* cid) {
    return Sd2Card_readRegister(sd2card, CMD10, cid);
}

bool Sd2Card_readCSD(Sd2Card_t *sd2card, csd_t* csd) {
    return Sd2Card_readRegister(sd2card, CMD9, csd);
}

void Sd2Card_error(Sd2Card_t *sd2card, uint8_t code) {
    sd2card->m_errorCode = code;
}

int Sd2Card_errorCode(Sd2Card_t *sd2card) {
    return sd2card->m_errorCode;
}

int Sd2Card_errorData(Sd2Card_t *sd2card) {
    return sd2card->m_status;
}

int Sd2Card_getType(Sd2Card_t *sd2card) {
    return sd2card->m_type;
}
