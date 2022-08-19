#ifndef EXLNK_SET_HEADER_H_
#define EXLNK_SET_HEADER_H_

#include "stdint.h"
#include "exlnk_options.h"

extern void exlnk_setHeader( uint16_t adr, uint8_t * trg, uint8_t type_msg, uint8_t type_pack, uint16_t len, uint32_t cnt, uint16_t ovrflw);
extern void exlnk_setEnder(uint8_t * trg);
#endif