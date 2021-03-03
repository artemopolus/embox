#ifndef SPI2_GENERATED_H
#define SPI2_GENERATED_H
#include <stdint.h>
extern uint8_t SPI2_FULL_DMA_transmit(uint8_t *data, uint8_t datacount);
extern uint8_t SPI2_FULL_DMA_receive(uint8_t *data, uint8_t datacount);
extern uint8_t SPI2_FULL_DMA_setdatalength( uint8_t datalength );
extern struct mutex SPI2_FULL_DMA_wait_rx_data(void);
extern uint8_t SPI2_FULL_DMA_is_full(void);
#endif //SPI2_GENERATED_H
