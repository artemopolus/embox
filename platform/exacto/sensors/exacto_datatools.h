#ifndef EXACTO_DATATOOLS_H
#define EXACTO_DATATOOLS_H
#include <stdint.h>

extern void ex_convertUint8ToInt16(uint8_t * src, int16_t * dst);
extern void ex_convertUint8ToUint16(uint8_t * src, uint16_t * dst);
extern void ex_convertUint8ToUint64(uint8_t * src, uint64_t * dst);

#endif //EXACTO_DATATOOLS_H