#ifndef BASIC_H
#define BASIC_H
#include <stdint.h>

extern uint8_t  ex_feedCRC(uint8_t * buffer, uint16_t buffer_length);
extern uint32_t ex_getResultCRC();
extern uint8_t  ex_getCRC(uint8_t * buffer, uint16_t buffer_length, uint32_t * result);

#endif 
