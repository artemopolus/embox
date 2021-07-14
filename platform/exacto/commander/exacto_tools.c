#include "commander/exacto_tools.h"
void ex_convertUint8ToInt16(uint8_t * src, int16_t * dst)
{
    int16_t first = (int16_t) src[1];
    *dst = (first << 8) + (int16_t)src[0];
}
void ex_convertUint8ToUint64(uint8_t * src, uint64_t * dst)
{
    *dst = 0;
    for (uint16_t i = 0; i < 4; i++)
        *dst += (uint64_t) (src[i] << i*8);

    // *dst =  (uint64_t) src[0];
    // *dst += (uint64_t) (src[1] << 8);
    // *dst += (uint64_t) (src[2] << 16);
    // *dst += (uint64_t) (src[3] << 24);
}
