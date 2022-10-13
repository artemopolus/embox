#ifndef EXACTO_CALC_H_
#define EXACTO_CALC_H_


#include <stdint.h>

extern uint8_t excc_setGetter(uint16_t(*receive)(uint8_t * data, const uint16_t datalen));

extern uint8_t excc_exeProcess();


#endif //EXACTO_CALC_H_