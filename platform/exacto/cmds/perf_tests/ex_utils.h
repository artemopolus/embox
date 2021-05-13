#ifndef EX_UTILS_H
#define EX_UTILS_H
#include <stdint.h>
extern void ex_dwt_cyccnt_reset(void);
extern uint32_t ex_dwt_cyccnt_start(void);
extern uint32_t ex_dwt_cyccnt_stop(void);

#endif
