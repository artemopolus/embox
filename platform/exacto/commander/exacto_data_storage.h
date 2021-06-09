#ifndef EXACTO_DATA_STORAGE_H
#define EXACTO_DATA_STORAGE_H
#include <errno.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "commander/exacto_buffer.h"
#include "commander/exacto_services.h"

#include <framework/mod/options.h>
#include <module/exacto/commander/data_storage.h>
#define MODOPS_EXACTOLINK_BUFFER_SZ OPTION_MODULE_GET(exacto__commander__data_storage, NUMBER, exactolinkbuffersz)

#if MODOPS_EXACTOLINK_BUFFER_SZ == 16
#define EXACTOLINK_MESSAGE_SIZE  16
#elif MODOPS_EXACTOLINK_BUFFER_SZ == 32
#define EXACTOLINK_MESSAGE_SIZE  32 
#elif MODOPS_EXACTOLINK_BUFFER_SZ == 64
#define EXACTOLINK_MESSAGE_SIZE  64
#elif MODOPS_EXACTOLINK_BUFFER_SZ == 128
#define EXACTOLINK_MESSAGE_SIZE  128 
#elif MODOPS_EXACTOLINK_BUFFER_SZ == 256
#define EXACTOLINK_MESSAGE_SIZE  256
#else
#error Unsupported exactolink buffer sz
#endif

#define EXACTOLINK_SD_FRAME_SIZE 128

#define THREAD_CONTROL_BUFFER_SZ 16
#define THREAD_OUTPUT_TYPES_SZ 4
#define SERVICES_COUNT 5

#define EXACTOLINK_START_DATA_POINT_ADR 4
#define EXACTOLINK_START_DATA_POINT_VAL 13
#define EXACTOLINK_PCK_ID 17

#include <stdint.h>
// typedef enum{
//     APPEND = 0,
//     GET,
//     CHECK
// }function_list_t;


typedef enum t_c_r_t{
    THR_CTRL_OK = 0,
    THR_CTRL_INIT,
    THR_CTRL_WAIT,
    THR_CTRL_READY,
    THR_CTRL_UNKNOWN_ERROR,
    THR_CTRL_NO_RESULT = 0xFF
}thread_control_result_t;
// THREADS



typedef struct {
    struct lthread thread;                          // поток исполнения запросов
    struct mutex mx;                             // контрольный мьютекс для контроля окончания потока
    // uint8_t databuffer[THREAD_CONTROL_BUFFER_SZ];   // буффер хранения данных
    ExactoBufferUint8Type datastorage;
    uint8_t datamaxcount;
    uint8_t datalen;
    thread_control_result_t result;
    thread_type_t type;
    uint8_t isready;
    // function_list_t fun_type;
}thread_control_t;

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
    EXACTOLINK_LSM303AH_TYPE0,
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
    exactolink_package_result_t packagetype;
}exactolink_package_info_t;


extern ex_subs_service_t ExDataStorageServices[SERVICES_COUNT];
extern ex_service_info_t ExDataStorageServicesInfo;

extern thread_control_t ExOutputStorage[THREAD_OUTPUT_TYPES_SZ]; 
extern exactodatastorage ExDtStorage;
extern ex_io_thread_t ExSpi;
extern uint8_t checkExactoDataStorage( thread_control_t * base );
extern uint8_t initThreadExactoDataStorage( thread_control_t * base );
extern uint8_t transmitExactoDataStorage();
extern uint8_t receiveExactoDataStorage();
extern uint8_t setupReceiveLengthExactoDataStorage( const uint8_t length);
extern uint8_t clearExactoDataStorage();
extern thread_control_result_t getStateExactoDataStorage();
extern uint8_t setDataToExactoDataStorage(uint8_t * data, const uint16_t datacount, thread_control_result_t result);
extern uint8_t getMailFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length);

extern exactolink_package_result_t ex_checkData_ExDtStr();
extern uint16_t ex_getData_ExDtStr(uint8_t * buffer, uint16_t buffer_length, uint16_t data_type);
extern uint16_t ex_pshBuf_ExDtStr(ExactoBufferUint8Type * buffer, uint16_t buffer_length, uint16_t data_type);
extern uint8_t ex_getInfo_ExDtStr(exactolink_package_info_t * info);
extern void ex_updateCounter_ExDtStr(thread_type_t type);
extern uint32_t ex_getCounter_ExDtStr(thread_type_t type);

extern uint8_t getDataFromExactoDataStorage(uint8_t * receiver, const uint8_t receiver_length);
extern uint8_t resetExactoDataStorage();
extern uint8_t checkTxSender();
extern uint8_t checkRxGetter();
extern void startTickReactionThread( );

extern uint8_t subscribeOnEvent(int (*run)(struct lthread *));
#endif //EXACTO_DATA_STORAGE_H
