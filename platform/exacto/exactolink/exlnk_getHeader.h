#ifndef EXLNK_GET_HEADER_H_
#define EXLNK_GET_HEADER_H_

#include "stdint.h"
#include "exlnk_options.h"

typedef struct exlnk_get_header_str{
    uint16_t adr;
    uint8_t type_msg;
    uint8_t type_pack;
    uint16_t len;
    uint32_t cnt;
    uint16_t ovrflw;
}exlnk_get_header_str_t;

extern uint8_t exlnk_getHeader( uint8_t * trg, uint16_t len, exlnk_get_header_str_t * out);

#endif