#ifndef EXACTO_DATA_STORAGE_H
#define EXACTO_DATA_STORAGE_H
#include <errno.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>

#include <stdint.h>
// typedef enum{
//     APPEND = 0,
//     GET,
//     CHECK
// }function_list_t;
typedef enum t_c_r_t{
    THR_CTRL_OK = 0,
    THR_CTRL_WAIT,
    THR_CTRL_UNKNOWN_ERROR,
    THR_CTRL_NO_RESULT = 0xFF
}thread_control_result_t;
// THREADS
typedef struct {
    struct lthread thread;                          // поток исполнения запросов
    struct mutex mx;                             // контрольный мьютекс для контроля окончания потока
    uint8_t databuffer[THREAD_CONTROL_BUFFER_SZ];   // буффер хранения данных
    uint8_t datalen;
    thread_control_result_t result;
    // function_list_t fun_type;
}thread_control_t;
typedef struct{
    uint8_t isEmpty;
    struct mutex dtmutex;
}exactodatastorage;
typedef enum{
    EXACTO_OK = 0,
    EXACTO_WAITING,
    EXACTO_PROCESSING,
    EXACTO_DENY,
    EXACTO_ERROR
}exacto_process_result_t;
extern exactodatastorage ExDtStorage;
extern uint8_t checkExactoDataStorage( thread_control_t * base );
extern uint8_t initThreadExactoDataStorage( thread_control_t * base );
#endif //EXACTO_DATA_STORAGE_H
