#ifndef EXACTO_MLINER_MAIN_MOD_H
#define EXACTO_MLINER_MAIN_MOD_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "commander/exacto_data_storage.h"
#include <embox/unit.h>

extern exactolink_package_info_t   TESMAF_ReceivedData_Info;
extern uint32_t TESMAF_Tx_Buffer;
extern uint32_t TESMAF_Rx_Buffer;
extern uint32_t TESMAF_DataCheck_Counter;
extern uint32_t TESMAF_DataCheck_Success;
extern uint32_t TESMAF_DataCheck_CntBuff;
extern uint32_t TESMAF_DataCheck_ScsBuff;
extern uint8_t TESMAF_Sync_Marker;
extern uint8_t TESMAF_Sensors_Marker;

extern uint8_t TESMAF_Sensors_TickCnt;
extern uint8_t TESMAF_Sensors_TickMax;
extern uint8_t TESMAF_Sensors_GoodCnt;
extern uint8_t TESMAF_Sensors_GoodMax;

extern uint8_t TESMAF_test_uploaddatamarker;
extern uint8_t TESMAF_test_pushtosdmarker;
extern uint8_t TESMAF_test_PushToSdMarkerGood;
extern uint8_t TESMAF_test_PushToSdMarkerBad ;

extern uint32_t TESMAF_test_CallFunTooManyFailed;
extern uint32_t TESMAF_test_InputLst;
extern uint32_t TESMAF_test_UpdteLst;

extern uint8_t setMlinerMode(const uint16_t address, exactolink_package_result_t mode);


#endif
