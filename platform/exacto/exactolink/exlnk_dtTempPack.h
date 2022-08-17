#ifndef EXLNK_DT_TEMP_PACK_H_
#define EXLNK_DT_TEMP_PACK_H_
#include "stdint.h"
#include "exlnk_options.h"


typedef struct exlnk_temp_str{
    uint32_t value;
}exlnk_temp_str_t;


extern void exlnk_setTempPack(uint8_t * trg, uint32_t value);

extern uint8_t exlnk_getTempPack(uint8_t * trg, const uint16_t len, uint32_t * value);

#endif
