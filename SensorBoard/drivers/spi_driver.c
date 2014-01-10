#include <avr/io.h>
#include <stddef.h>

#include "spi_driver.h"

/*! \brief Initialize SPI module as master.
 *
 *  This function initializes a SPI module as master. The CTRL and INTCTRL
 *  registers for the SPI module is set according to the inputs to the function.
 *  In addition, data direction for the MOSI and SCK pins is set to output.
 *
 *  \param spi            The SPI_Master_t struct instance.
 *  \param module         The SPI module.
 *  \param port           The I/O port where the SPI module is connected.
 *  \param lsbFirst       Data order will be LSB first if this is set to a
 *                        non-zero value.
 *  \param mode           SPI mode (Clock polarity and phase).
 *  \param intLevel       SPI interrupt level.
 *  \param clk2x	      SPI double speed mode
 *  \param clockDivision  SPI clock pre-scaler division factor.
 */
void SPI_MasterInit(SPI_Master_t *spi,
                    SPI_t *module,
                    PORT_t *port,
                    bool lsbFirst,
                    SPI_MODE_t mode,
                    bool clk2x,
                    SPI_PRESCALER_t clockDivision)
{
    spi->module         = module;
    spi->port           = port;

    spi->module->CTRL   = clockDivision |                  /* SPI prescaler. */
                            (clk2x ? SPI_CLK2X_bm : 0) |     /* SPI Clock double. */
                            SPI_ENABLE_bm |                  /* Enable SPI module. */
                            (lsbFirst ? SPI_DORD_bm  : 0) |  /* Data order. */
                            SPI_MASTER_bm |                  /* SPI master. */
                            mode;                            /* SPI mode. */

    // MOSI and SCK as output.
    spi->port->DIRSET  = SPI_MOSI_bm | SPI_SCK_bm;
}


/*! \brief SPI mastertransceive byte
 *
 *  This function clocks data in the DATA register to the slave, while data
 *  from the slave is clocked into the DATA register. The function does not
 *  check for ongoing access from other masters before initiating a transfer.
 *  For multimaster systems, checkers should be added to avoid bus contention.
 *
 *  SS line(s) must be pulled low before calling this function and released
 *  when finished.
 *
 *  \note This function is blocking and will not finish unless a successful
 *        transfer has been completed. It is recommended to use the
 *        interrupt-driven driver for applications where blocking
 *        functionality is not wanted.
 *
 *  \param spi        The SPI_Master_t struct instance.
 *  \param TXdata     Data to transmit to slave.
 *
 *  \return           Data received from slave.
 */
uint8_t SPI_MasterTransceiveByte(SPI_Master_t *spi, uint8_t TXdata)
{
	// Send pattern.
	spi->module->DATA = TXdata;

	// Wait for transmission complete.
	while (!(spi->module->STATUS & SPI_IF_bm)) {
        // Do nothing
	}
    
	// Read received data.
	return spi->module->DATA;
}

inline uint8_t SPI_MasterReceiveByte(SPI_Master_t *spi)
{
    return SPI_MasterTransceiveByte(spi, 0xFF);
}

