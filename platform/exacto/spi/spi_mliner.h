#ifndef SPI_MLINER_H
#define SPI_MLINER_H
#include <stdint.h>

#define SPI_MLINER_BUFFER_SIZE EXACTOLINK_MESSAGE_SIZE

extern void SPI1_FULL_DMA_SetEnabled();
extern void disableMasterSpiDma();
extern void enableMasterSpiDma();

extern void syncMasterSpiDma();

extern uint8_t SPI2_FULL_DMA_setdatalength( uint8_t datalength );
extern struct mutex SPI2_FULL_DMA_wait_rx_data(void);
extern uint8_t SPI2_FULL_DMA_is_full(void);
extern void setupSPI2_FULL_DMA();
extern void turnOffSPI2_FULL_DMA();
#endif //
