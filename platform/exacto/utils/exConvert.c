#include "exConvert.h"

void ex_conv_floatTOuint8(float src, uint8_t * trg)
{
    uint32_t fbits = 0;
    memcpy(&fbits, &src, sizeof fbits);
    trg[0] = (uint8_t)fbits;
    trg[1] = (uint8_t)(fbits >> 8);
    trg[2] = (uint8_t)(fbits >> 16);
    trg[3] = (uint8_t)(fbits >> 24);
}
void ex_conv_uint8TOfloat(uint8_t * src, float * trg)
{
    uint32_t fbits = (uint32_t)src[0];
    fbits += (uint32_t)(src[1] << 8);
    fbits += (uint32_t)(src[2] << 16);
    fbits += (uint32_t)(src[3] << 24);
    float val;
    memcpy(&val, &fbits, sizeof fbits);
    *trg = val;
}

