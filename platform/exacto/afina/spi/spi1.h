#ifndef SPI1_H
#define SPI1_H
#include <stdint.h>
extern uint8_t SPI1_FULL_DMA_setdatalength( uint8_t datalength );
extern struct mutex SPI1_FULL_DMA_wait_rx_data(void);
extern uint8_t SPI1_FULL_DMA_is_full(void);
#endif //SPI1_H
