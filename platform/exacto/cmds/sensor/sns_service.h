#ifndef SNS_SERVICE_H
#define SNS_SERVICE_H
#include <stdint.h>
typedef enum{
    EX_SNSS_SMPL = 0,
    EX_SNSS_ONLY_ACC
}exacto_sns_service_status_t;
extern uint8_t ex_setFreqHz_SnsService(const uint16_t freq, const exacto_output_state_t state);
#endif
