#ifndef OW_H_
#define OW_H_

#include <stdint.h>

#define EX_OW_PACK_SZ 32

typedef struct{
    uint8_t data[EX_OW_PACK_SZ];
    uint8_t datalen;
    uint8_t cmd;
}ex_ow_pack_t;

extern uint8_t ex_onewire_reset(void );

extern uint8_t ex_onewire_send(ex_ow_pack_t * out);

extern uint8_t ex_onewire_read(ex_ow_pack_t * in);

#endif