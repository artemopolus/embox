#ifndef SPI_MLINE_SEC_H_
#define SPI_MLINE_SEC_H_


#define SPI_MLINE_RXTX_BUFFER_SIZE 512

extern void setTxBuffSpiDevSec(ExactoBufferUint8Type * buffer);
extern void setRxBuffSpiDevSec(ExactoBufferUint8Type * buffer);
extern void receiveSpiDevSec();
extern uint8_t repeatTransmitSpiDevSec();
extern uint8_t transmitSpiDevSec();
extern void enableSpiDevSec();
extern void disableSpiDevSec();

extern uint16_t getReceivedDataSpiDevSec(uint8_t * trg, uint16_t trglen);
extern uint16_t setTransmitDataSpiDevSec(uint8_t * src, uint16_t srclen);

#endif