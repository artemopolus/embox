#ifndef EX_CONVERT_H_
#define EX_CONVERT_H_

#include "stdint.h"

extern void ex_conv_floatTOuint8(float src, uint8_t * trg);
extern void ex_conv_uint8TOfloat(uint8_t * src, float * trg);

#endif
