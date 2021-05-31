#ifndef EXACTO_SNS_CTRL_H
#define EXACTO_SNS_CTRL_H
#include "spi/spi_sns.h"
#include <stdint.h>
#define EX_SNS_CMDS_COUNT 6
typedef enum {
    LSM303AH = 0,
    ISM330DLC,
    BMP280
}exacto_sensors_list_t;
typedef struct{
    exacto_sensors_list_t sns;
    uint8_t isenabled;
    uint8_t address;
    uint16_t datalen;
    uint16_t pt2buffer;
    uint8_t shift;
}ex_sns_cmds_t;
typedef struct{
    struct lthread thread;
    uint8_t sns_count;
    ex_sns_cmds_t sns[EX_SNS_CMDS_COUNT];
    uint8_t sns_current;
}ex_sns_lth_container_t;
// const uint8_t lsm303ah_3wire_adr = 0x21;
    // const uint8_t lsm303ah_3wire_val = 0x07;
    // const uint8_t lsm303ah_whoami_xl_adr = 0x0f;
    // const uint8_t lsm303ah_whoami_xl_val = 0x43;
    // const uint8_t lsm303ah_whoami_mg_adr = 0x4f;
    // const uint8_t lsm303ah_whoami_mg_val = 0x40; // or 41
    // const uint8_t ism330dlc_3wire_adr = 0x12;
    // const uint8_t ism330dlc_3wire_val = 0x0C;
    // const uint8_t ism330dlc_whoami_adr = 0x0F;
    // const uint8_t ism330dlc_whoami_val = 0x6A;
    // const uint8_t lsm303ah_ctrl1 = 0x20;
    // const uint8_t lsm303ah_ctrl1_value = 0x39; // 0xb4; //10110100

#define LSM303AH_3WIRE_ADR 0x21
#define LSM303AH_3WIRE_VAL 0x07
#define LSM303AH_WHOAMI_XL_ADR 0x0f
#define LSM303AH_WHOAMI_XL_VAL 0x43
#define LSM303AH_WHOAMI_MG_ADR 0x4f
#define LSM303AH_WHOAMI_MG_VAL 0x40
#define LSM303AH_CTRL1_ADR 0x20
#define LSM303AH_CTRL1_VAL 0x39
#define LSM303AH_STATUS_A_ADR 0x28

#define ISM330DLC_WHOAMI_ADR 0x0f


extern void enableExactoSensor(exacto_sensors_list_t sensor);
extern void disableExactoSensor(exacto_sensors_list_t sensor);
extern uint8_t isDataReady_xl(exacto_sensors_list_t sensor, const uint8_t value);
extern uint8_t isDataReady_gr(exacto_sensors_list_t sensor, const uint8_t value);
extern uint8_t isDataReady_mg(exacto_sensors_list_t sensor, const uint8_t value);

extern uint8_t isXlGrDataReady_ISM330DLC(const uint8_t value);
extern uint8_t isXlGrDataReady(exacto_sensors_list_t sensor, const uint8_t value);
extern void convertUint8ToInt16(uint8_t * src, int16_t * dst);
extern void convertUint8ToUint64(uint8_t * src, uint64_t * dst);

#endif //EXACTO_SENSORS_H