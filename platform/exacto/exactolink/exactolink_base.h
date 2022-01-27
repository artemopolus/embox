#ifndef EXACTO_EXACTOLINK_EXACTOLINK_BASE_H
#define EXACTO_EXACTOLINK_EXACTOLINK_BASE_H

#include "commander/exacto_buffer.h"

#define EXACTOLINK_SDPACK_ID 0x11

void exlnk_initSDpack(const uint8_t type, const uint16_t cnt, const uint32_t refcnt, ExactoBufferExtended * buffer);
uint8_t exlnk_pushtoSDpack(const uint8_t value, ExactoBufferExtended * buffer);
#endif