#include "exacto_services.h"

uint8_t ex_subscribeOnEvent(ex_service_info_t * info, ex_subs_service_t * service,
 ex_thread_type_t type ,int (*run)(struct lthread *))
{
    uint8_t i = info->current_count;
    if (i == (info->max_count))
        return 1;
    service[i].isenabled = 1;
    service[i].type = type;
    service[i].done = 1;
    lthread_init(&service[i].thread, run);
    info->current_count++;
    return 0;
}
void ex_updateEventForSubs(ex_service_info_t info, ex_subs_service_t * service, 
    ex_thread_type_t type)
{
   for (uint8_t i = 0; i < info.current_count; i++)
   {
       if ((service[i].type == type)&&(service[i].isenabled))
       {
           if (service[i].done)
           {
               lthread_launch(&service[i].thread);
               service[i].done = 0;
           }
       }
   }
    
}
void ex_initSubscribeEvents(ex_service_info_t info, ex_subs_service_t * service)
{
   for (uint8_t i = 0; i < info.max_count; i++)
   {
       service[i].isenabled = 0;
       service[i].type = EX_THR_NONE;
   }

}
uint8_t ex_initServiceMsg( ex_service_transport_msg_t * msg)
{
    for (uint8_t i = 0; i < EX_SERVICE_TRANSPORT_MSG_SZ; i++)
    {
        msg->data[i] = 0;
    }
    msg->isfull = 0; 
    msg->datalen = 0;
    return 0;
}
uint8_t ex_uploadDataToServiceMsg( ex_service_transport_msg_t * msg, uint8_t * data, const uint16_t datalen)
{
    if (datalen > EX_SERVICE_TRANSPORT_MSG_SZ)
        return 1;
    for (uint8_t i = 0; i < datalen; i++)
    {
        msg->data[i] = data[i];
    }
    msg->datalen = datalen;
    msg->isfull = 1; 
    return 0;
}
uint8_t ex_downloadDataFromServiceMsg( ex_service_transport_msg_t * msg, uint8_t * dst, const uint16_t dstlen)
{
    if (!msg->isfull)
        return 1;
    for (uint8_t i = 0; ((i < dstlen)&&(i < EX_SERVICE_TRANSPORT_MSG_SZ)); i++)
    {
        dst[i] = msg->data[i];
    }
    msg->isfull = 0;
    return 0;
}
uint8_t ex_checkServiceMsg(ex_service_transport_msg_t * msg)
{
    return msg->isfull;
}
uint16_t ex_getLenServiceMsg(ex_service_transport_msg_t * msg)
{
    if (!msg->isfull)
        return 0;
    return msg->datalen;
}
