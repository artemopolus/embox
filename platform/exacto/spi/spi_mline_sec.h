#ifndef SPI_MLINE_SEC_H_
#define SPI_MLINE_SEC_H_


#define SPI_MLINE_RXTX_BUFFER_SIZE 512

extern void setTxBuffSpiDevSec(ExactoBufferUint8Type * buffer);
extern void setRxBuffSpiDevSec(ExactoBufferUint8Type * buffer);
extern void receiveSpiDevSec();
extern void transmitSpiDevSec();
extern void enableSpiDevSec();
extern void disableSpiDevSec();

extern uint8_t getReceivedDataSpiDevSec(uint8_t * trg, uint16_t trglen);
extern uint8_t setTransmitDataSpiDevSec(uint8_t * src, uint16_t srclen);

#endif