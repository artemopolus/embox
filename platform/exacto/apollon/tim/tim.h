#ifndef TIM3_H
#define TIM3_H
#include <stdint.h>
#include "commander/exacto_services.h"

#define TIM_SERVICES_COUNT 3

extern ex_subs_service_t ExTimServices[TIM_SERVICES_COUNT];
extern ex_service_info_t ExTimServicesInfo;
extern void ex_setFreqHz(const uint32_t target_freq);
extern void ex_frcTimReload();


#endif //TIM3_H
