#ifndef EXACTO_SERVICES_H_
#define EXACTO_SERVICES_H_
#include <stdint.h>
#include <errno.h>
#include <kernel/lthread/lthread.h>

#define EX_SERVICE_TRANSPORT_MSG_SZ 8
#define EX_SERVICE_TRANSPORT_PCK_SZ 64
typedef enum t_t_t{
    EX_THR_SPi_RX = 0,
    EX_THR_SPi_TX,
    EX_THR_STR_CALC_IN,
    EX_THR_STR_CALC_OUT,
    // EX_THR_STR_SD,
    EX_THR_I2C_RX,
    EX_THR_I2C_TX,
    EX_THR_TIM,
    EX_THR_NONE
}ex_thread_type_t;

typedef struct {
    uint8_t data[EX_SERVICE_TRANSPORT_MSG_SZ];
    uint16_t datalen;
    uint8_t isfull;
}ex_service_transport_msg_t;


typedef struct {
    struct lthread thread;
    ex_thread_type_t type;
    uint8_t isenabled;
    uint8_t done;
}ex_subs_service_t;

typedef struct {
    uint8_t current_count;
    uint8_t max_count;
}ex_service_info_t;
extern uint8_t ex_subscribeOnEvent(ex_service_info_t * info, ex_subs_service_t * service,
 ex_thread_type_t type ,int (*run)(struct lthread *));
extern void ex_updateEventForSubs(ex_service_info_t info, ex_subs_service_t * service, 
    ex_thread_type_t type);

extern int exse_subscribe(ex_service_info_t * info, ex_subs_service_t * service,
 ex_thread_type_t type ,int (*run)(struct lthread *));
extern void exse_ack(ex_subs_service_t * service);

extern void ex_initSubscribeEvents(ex_service_info_t  info, ex_subs_service_t * service);

extern uint8_t ex_initServiceMsg( ex_service_transport_msg_t * msg);
extern uint8_t ex_uploadDataToServiceMsg( ex_service_transport_msg_t * msg, uint8_t * data, const uint16_t datalen);
extern uint8_t ex_downloadDataFromServiceMsg( ex_service_transport_msg_t * msg, uint8_t * dst, const uint16_t dstlen);
extern uint8_t ex_checkServiceMsg(ex_service_transport_msg_t * msg);
extern uint16_t ex_getLenServiceMsg(ex_service_transport_msg_t * msg);
#endif
