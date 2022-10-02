#ifndef EX_UTILS_H
#define EX_UTILS_H
#include <stdint.h>

typedef struct exutils_data{
	uint32_t start; 
	uint32_t stop; 
	uint32_t result;
	uint8_t is_enabled;
}exutils_data_t;

extern void ex_dwt_cyccnt_reset(void);
extern uint32_t ex_dwt_cyccnt_start(void);
extern uint32_t ex_dwt_cyccnt_stop(void);

extern void exutils_init(exutils_data_t * trg);
extern void exutils_updt(exutils_data_t * trg);

#endif
