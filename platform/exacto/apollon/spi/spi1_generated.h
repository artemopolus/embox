#ifndef SPI1_GENERATED_H
#define SPI1_GENERATED_H
#include <stdint.h>
#include "exacto_commander/exacto_data_storage.h"
#include "project/base_project_defs.h"
typedef enum{
    SPI_DT_TRANSMIT = 0,
    SPI_DT_RECEIVE,
    SPI_DT_CHECK,
    SPI_DT_TRANSMIT_RECEIVE,
    SPI_DT_SET
}spi_data_type_t;
typedef struct{
    uint8_t data[SPI_PACK_SZ];
    uint8_t datalen;
    spi_data_type_t type;
    exacto_process_result_t result;
}spi_pack_t;
extern uint8_t SPI1_HALF_BASE_get_option(const uint8_t address);
extern uint8_t SPI1_HALF_BASE_set_option(const uint8_t address, const uint8_t value);
extern uint8_t sendSpi1Half(spi_pack_t * input);
extern uint8_t waitSpi1Half(spi_pack_t *output);
#endif //SPI1_GENERATED_H
