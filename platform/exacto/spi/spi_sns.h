#ifndef SPI_SNS_H
#define SPI_SNS_H
#include <stdint.h>
#include "commander/exacto_services.h"

#define EX_SPI_SERVICES_COUNT 2
#define EX_SPI_PACK_SZ 32 
typedef enum{
    EXACTO_OK = 0,
    EXACTO_INIT,
    EXACTO_WAITING,
    EXACTO_PROCESSING,
    EXACTO_DENY,
    EXACTO_ERROR
}exacto_process_result_t;


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
    uint8_t cmd;
    ex_spi_data_type_t type;
    exacto_process_result_t result;
}ex_spi_pack_t;
extern uint8_t ex_sendSpiSns(ex_spi_pack_t *  input);
extern uint8_t ex_gettSpiSns(ex_spi_pack_t * output);
extern uint8_t ex_runReceiver();
extern uint8_t ex_runTransmiter();
#endif //SPI_SNS_H
