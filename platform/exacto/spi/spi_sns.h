#ifndef SPI_SNS_H
#define SPI_SNS_H
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_services.h"

#define EX_SPI_SERVICES_COUNT 2
#define EX_SPI_PACK_SZ 16

extern ex_subs_service_t ExSnsServices[EX_SPI_SERVICES_COUNT];
extern ex_service_info_t ExSnsServicesInfo;

typedef enum{
    EX_SPI_DT_TRANSMIT = 0,
    EX_SPI_DT_RECEIVE,
    EX_SPI_DT_CHECK,
    EX_SPI_DT_TRANSMIT_RECEIVE,
    EX_SPI_DT_SET
}ex_spi_data_type_t;
typedef struct{
    uint8_t data[EX_SPI_PACK_SZ];
    uint8_t datalen;
    ex_spi_data_type_t type;
    exacto_process_result_t result;
}ex_spi_pack_t;
extern uint8_t ex_getSpiSnsOption(const uint8_t address);
extern uint8_t ex_setSpiSnsOption(const uint8_t address, const uint8_t value);
extern uint8_t ex_sendSpiSns(ex_spi_pack_t *  input);
extern uint8_t ex_waitSpiSns(ex_spi_pack_t *  output);
extern uint8_t ex_runReceiver();
#endif //SPI_SNS_H
