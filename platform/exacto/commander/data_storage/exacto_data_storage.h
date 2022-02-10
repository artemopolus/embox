#ifndef EXACTO_DATA_STORAGE_H
#define EXACTO_DATA_STORAGE_H
#include <errno.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "commander/exacto_buffer.h"
#include "commander/exacto_services.h"
#include "commander/exacto_tools.h"

#include "commander/exacto_data_storage_options.h"

#include "exactolink/exactolink_base.h"

#include <stdint.h>
// typedef enum{
//     APPEND = 0,
//     GET,
//     CHECK
// }function_list_t;


typedef enum t_c_r_t{
    EX_THR_CTRL_OK = 0,
    EX_THR_CTRL_INIT,
    EX_THR_CTRL_WAIT,
    EX_THR_CTRL_READY,
    EX_THR_CTRL_OVRFL,
    EX_THR_CTRL_UNKNOWN_ERROR,
    EX_THR_CTRL_NO_RESULT = 0xFF
}ex_thread_control_result_t;
// THREADS



typedef struct {
    struct lthread thread;                          // поток исполнения запросов
    struct mutex mx;                             // контрольный мьютекс для контроля окончания потока
    // uint8_t databuffer[THREAD_CONTROL_BUFFER_SZ];   // буффер хранения данных
    ExactoBufferUint8Type datastorage;
    uint8_t datamaxcount;
    uint8_t datalen;
    ex_thread_control_result_t result;
    ex_thread_type_t type;
    uint8_t isready;
    // function_list_t fun_type;
}ex_thread_control_t;

typedef struct{
    struct lthread thread;
    uint8_t isready;
    uint8_t isenabled;
}ex_io_thread_t;
typedef struct{
    uint8_t isEmpty;
    struct mutex dtmutex;
}exactodatastorage;
typedef enum{
    EXACTO_OK = 0,
    EXACTO_INIT,
    EXACTO_WAITING,
    EXACTO_PROCESSING,
    EXACTO_DENY,
    EXACTO_ERROR
}exacto_process_result_t;

typedef enum{
    EXACTOLINK_NO_DATA = 0,
    EXACTOLINK_OK,
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

typedef struct{
    uint16_t length;
    uint8_t length_raw[2];
    uint16_t datatype;
    uint8_t priority;
    uint16_t datasrc;
    uint32_t counter;
    uint8_t counter_raw[4];
    uint8_t is_data_available;
    uint16_t overflow;
    exactolink_package_result_t packagetype;
    exacto_process_result_t status;
}exactolink_package_info_t;

// переменные

extern uint8_t EDS_spidmairq_Marker;

extern ex_subs_service_t ExDataStorageServices[SERVICES_COUNT];
extern ex_service_info_t ExDataStorageServicesInfo;

extern ex_thread_control_t ExDtStr_Output_Storage[THREAD_OUTPUT_TYPES_SZ]; 

extern ExactoBufferExtended ExDtStr_SD_buffer;

extern exactodatastorage ExDtStorage;
extern ex_io_thread_t ExSpi;
extern uint32_t ExDtStr_TrasmitSPI_LostCnt;
extern uint32_t ExDtStr_TrasmitSPI_DbleCnt;
extern uint32_t ExDtStr_TrasmitSPI_OverFlw;
extern uint16_t ExDtStr_OutputSPI_OverFlw;
extern uint32_t ExDtStr_TransmitSPI_TxCounter;
extern uint32_t ExDtStr_TransmitSPI_RxCounter;

// функции
extern uint8_t ex_setExactolinkType( exactolink_package_result_t new_type);
//
extern uint8_t checkExactoDataStorage( ex_thread_control_t * base );
extern uint8_t initThreadExactoDataStorage( ex_thread_control_t * base );
extern uint8_t transmitExactoDataStorage();
extern uint8_t receiveExactoDataStorage();
extern uint8_t setupReceiveLengthExactoDataStorage( const uint8_t length);
extern uint8_t clearExactoDataStorage();

extern uint8_t setDataToExactoDataStorage(uint8_t * data, const uint16_t datacount, ex_thread_control_result_t result);
extern uint8_t exds_setSnsData(const uint8_t sns_id, uint8_t * data, const uint16_t datacount);

extern uint8_t watchPackFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length, uint8_t type);

extern exactolink_package_result_t ex_checkData_ExDtStr();
extern uint16_t ex_getRawFromSD_ExDtStr(uint8_t * trg, const uint16_t copylen);
#ifdef EXDTSTR_EXTENDED
extern uint16_t ex_pshBuf_ExDtStr(ExactoBufferUint8Type * buffer, uint16_t buffer_length, uint16_t data_type);
extern uint16_t ex_getData_ExDtStr(uint8_t * buffer, uint16_t buffer_length, uint16_t data_type);
extern ex_thread_control_result_t  getStateExactoDataStorage();
#endif
extern void ex_updateCounter_ExDtStr(ex_thread_type_t type);

extern uint8_t resetExactoDataStorage();
extern uint8_t checkTxSender();
extern uint8_t checkRxGetter();
extern void startTickReactionThread( );

extern uint8_t subscribeOnEvent(int (*run)(struct lthread *));

extern uint8_t  ex_getInfo_ExDtStr(exactolink_package_info_t * info);
extern uint32_t ex_getCounter_ExDtStr(ex_thread_type_t type);
extern uint8_t  ex_getExactolinkType( exactolink_package_result_t * type);
extern uint16_t ex_getLength_ExDtStr(ex_thread_type_t type);
extern uint8_t  ex_getRawDataStr_ExDtStr( int16_t * dst, const uint16_t dstlen);

extern uint8_t                  getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length);
extern uint8_t                  getMailFromExactoDataStorage(uint8_t * receiver, const uint16_t receiver_length);
#endif //EXACTO_DATA_STORAGE_H
