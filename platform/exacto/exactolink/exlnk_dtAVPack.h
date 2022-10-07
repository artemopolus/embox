#ifndef EXLNK_DT_AV_PACK_H_
#define EXLNK_DT_AV_PACK_H_
#include "stdint.h"
#include "exlnk_options.h"


typedef struct exlnk_av_str{
    uint32_t A;
    uint32_t V;
}exlnk_av_str_t;

extern void exlnk_setAVPack( uint8_t * trg, exlnk_av_str_t value);
extern uint8_t exlnk_getAVPack( uint8_t * trg, exlnk_av_str_t * value);



#endif
