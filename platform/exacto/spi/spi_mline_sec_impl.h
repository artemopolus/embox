#ifndef SPI_MLINE_SEC_IMPL_H_
#define SPI_MLINE_SEC_IMPL_H_

#include "spi_mline_base.h"

extern int initBoardSpi(void);

extern void setBoardSpiBuffer(
	spi_mline_dev_t * transmit, spi_mline_dev_t * receive
	// uint8_t * rxdata, uint32_t rxlen, uint8_t * txdata, uint32_t txlen, int (*download)(uint32_t len), int(*upload)(uint32_t len)
	);

extern uint8_t runBoardSpiIRQhandlerTX(void);
extern uint8_t runBoardSpiIRQhandlerRX(void);

extern void enableBoardSpi(uint32_t rxlen, uint32_t txlen);
extern void disableBoardSpi(void);

extern void receiveBoardSpi(spi_mline_dev_t * receiver);

extern void resetBoardSpiRx(spi_mline_dev_t * receiver);
extern void resetBoardSpiRxTx(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit);

extern void receiveTransmitBoardSpi(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit);

#endif