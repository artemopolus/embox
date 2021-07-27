#ifndef TIM3_H
#define TIM3_H
#include <stdint.h>
#include "commander/exacto_services.h"
#include "commander/exacto_data_storage.h"

#define TIM_SERVICES_COUNT 3
typedef enum{
    EXACTO_TIM_10,
    EXACTO_TIM_50,
    EXACTO_TIM_100,
    EXACTO_TIM_200,
    EXACTO_TIM_400,
    EXACTO_TIM_800,
    EXACTO_TIM_1000,
    EXACTO_TIM_1600,
    EXACTO_TIM_2000,
    EXACTO_TIM_3200,
    EXACTO_TIM_6400
}exacto_tim_states_t;

extern ex_subs_service_t ExTimServices[TIM_SERVICES_COUNT];
extern ex_service_info_t ExTimServicesInfo;

extern void                 ex_setFreqHz(const uint32_t target_freq);
extern void                 ex_frcTimReload();
extern exacto_tim_states_t  ex_getFreqHz_TIM();


#endif //TIM3_H
