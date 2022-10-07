#ifndef EXACTO_MLINER_MAIN_MOD_H
#define EXACTO_MLINER_MAIN_MOD_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "commander/exacto_data_storage.h"
#include <embox/unit.h>


typedef enum{
	MLINER_M_NONE = 0,
	MLINER_M_STOP_MLINE,
	MLINER_M_STOP_ALL,
	MLINER_M_FIRST_START,
}mliner_main_mod_modes_t;

extern exactolink_package_info_t   TESMAF_ReceivedData_Info;
// extern uint32_t TESMAF_Tx_Buffer;
// extern uint32_t TESMAF_Rx_Buffer;
// extern uint32_t TESMAF_DataCheck_Counter;
// extern uint32_t TESMAF_DataCheck_Success;
// extern uint32_t TESMAF_DataCheck_CntBuff;
// extern uint32_t TESMAF_DataCheck_ScsBuff;
// extern uint32_t TESMAF_test_CallFunTooManyFailed;
// extern uint32_t TESMAF_test_InputLst;
// extern uint32_t TESMAF_test_UpdteLst;

typedef enum mmmvt{
	TX_BUFFER = 0,
	RX_BUFFER,
	DATACHECK_COUNTER,
	DATACHECK_SUCCESS,
	DATACHECK_CNTBUFF,
	DATACHECK_SCSBUFF,
	TEST_CALLFUNTOOMANYFAILED,
	TEST_INPUTLST,
	TEST_UPDTELST
}mliner_main_mod_vars_t;


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


extern uint8_t setMlinerMode(const uint16_t address, exactolink_package_result_t mode);

extern uint8_t startMliner(void);

extern uint8_t stopMliner(void);

extern void getMlinerVars(mliner_main_mod_vars_t type, uint32_t * ptr, uint8_t * check);

#endif
