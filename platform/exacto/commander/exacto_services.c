#include "exacto_services.h"

uint8_t ex_subscribeOnEvent(ex_service_info_t info, ex_subs_service_t * service,
 thread_type_t type ,int (*run)(struct lthread *))
{
    uint8_t i = info.current_count;
    if (i == (info.max_count -1))
        return 1;
    service[i].isenabled = 1;
    service[i].type = type;
    lthread_init(&service[i].thread, run);
    info.current_count++;
    return 0;
}
void ex_updateEventForSubs(ex_service_info_t info, ex_subs_service_t * service, 
    thread_type_t type)
{
   for (uint8_t i = 0; i < info.current_count; i++)
   {
       if ((service[i].type == type)&&(service[i].isenabled))
       {
           lthread_launch(&service[i].thread);
       }
   }
    
}