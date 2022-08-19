#include "exlnk_dtTempPack.h"


void exlnk_setTempPack(uint8_t * trg, uint32_t value)
{
    for (uint8_t i = 0; i < 4; i++)
        trg[i] = (uint8_t)(value >> i*8);
}

uint8_t exlnk_getTempPack(uint8_t * trg, const uint16_t len, uint32_t * value)
{
    for(uint8_t i = 0; i < 4; i++)
        *value += (uint32_t)(trg[i] << i*8);
    return 1;
}

