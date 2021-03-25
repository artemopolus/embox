#ifndef EXACTO_SERVICES_H
#define EXACTO_SERVICES_H
#include <stdint.h>
#include <errno.h>
#include <kernel/lthread/lthread.h>
typedef enum t_t_t{
    THR_SPI_RX = 0,
    THR_SPI_TX,
    THR_I2C_RX,
    THR_I2C_TX,
    THR_TIM,
    THR_NONE
}thread_type_t;


typedef struct {
    struct lthread thread;
    thread_type_t type;
    uint8_t isenabled;
}ex_subs_service_t;

typedef struct {
    uint8_t current_count;
    uint8_t max_count;
}ex_service_info_t;
extern uint8_t ex_subscribeOnEvent(ex_service_info_t * info, ex_subs_service_t * service,
 thread_type_t type ,int (*run)(struct lthread *));
extern void ex_updateEventForSubs(ex_service_info_t info, ex_subs_service_t * service, 
    thread_type_t type);
extern void ex_initSubscribeEvents(ex_service_info_t  info, ex_subs_service_t * service);
#endif