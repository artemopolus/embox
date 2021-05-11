#include "exacto_datatools.h"

void ex_convertUint8ToInt16(uint8_t * src, int16_t * dst)
{
    int16_t first = (int16_t) src[1];
    *dst = (first << 8) + (int16_t)src[0];
}
void ex_convertUint8ToUint64(uint8_t * src, uint64_t * dst)
{
    *dst =  (uint64_t) src[0];
    *dst += (uint64_t) (src[1] << 8);
    *dst += (uint64_t) (src[2] << 16);
    *dst += (uint64_t) (src[3] << 24);
}
