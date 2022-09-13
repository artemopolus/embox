#ifndef EXACTO_SENSORS_CTRL_H
#define EXACTO_SENSORS_CTRL_H
#include <stdint.h>
#include <kernel/lthread/lthread.h>
#define EX_SNS_CMDS_COUNT 6
typedef enum exacto_sensor_list{
    LSM303AH = 1,
    ISM330DLC = 2,
    BMP280 = 4
}exacto_sensors_list_t;
typedef struct ex_sns_cmd{
    exacto_sensors_list_t sns;
    uint8_t isenabled;
    uint8_t address;
    uint16_t datalen;
    uint16_t pt2buffer;
    uint8_t shift;
    uint32_t counter;
    uint8_t cnt_cur;
    uint8_t cnt_max;
    uint8_t dtrd;
}ex_sns_cmds_t;
typedef struct ex_sns_lth_container{
    struct lthread thread;
    uint8_t sns_count;
    ex_sns_cmds_t sns[EX_SNS_CMDS_COUNT];
    uint8_t sns_current;
    uint8_t done;
}ex_sns_lth_container_t;

typedef struct ex_ctrl_datapack
{
    uint32_t LostCnt;
    uint32_t DbleCnt;
    uint32_t OverFlw;
    uint32_t TxCounter;
    uint32_t RxCounter;
}ex_ctrl_datapack_t;





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
typedef enum{
    EXACTOLINK_NO_DATA = 0,
    EXACTOLINK_OK,
    EXACTOLINK_REPEAT,
    EXACTOLINK_LSM303AH_TYPE0,
    EXACTOLINK_LSM303AH_TYPE1,
    EXACTOLINK_LSM303AH_TYPE3,
    EXACTOLINK_CMD_COMMON,
    EXACTOLINK_CMD_SEND,
    EXACTOLINK_SNS_XLXLGR,
    EXACTOLINK_SNS_XL_0100_XLGR_0100,
    EXACTOLINK_SNS_XL_0200_XLGR_0100,
    EXACTOLINK_SNS_XL_0400_XLGR_0100,
    EXACTOLINK_SNS_XL_0800_XLGR_0100,
    EXACTOLINK_SNS_XL_1600_XLGR_0100,
    EXACTOLINK_SNS_XL_0200_XLGR_0200,
    EXACTOLINK_SNS_XL_0400_XLGR_0400,
    EXACTOLINK_CRC_ERROR,
    EXACTOLINK_UNKNOWN_ERROR
}exactolink_package_result_t;


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