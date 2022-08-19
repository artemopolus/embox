#include "exlnk_setHeader.h"

void exlnk_setHeader( uint16_t adr, uint8_t * trg, uint8_t type_msg, uint8_t type_pack, uint16_t len, uint32_t cnt, uint16_t ovrflw)
{
    //header
    trg[0] = EXLNK_PCK_ID;
    uint16_t length = type_pack + len + 4;
    trg[1] = (uint8_t) (length);
    trg[2] = (uint8_t) (length >> 8);
    //body header
    trg[3] = type_msg;
    trg[4] = type_pack; //datatype
    trg[5] = 0;
    trg[6] = 0xff;  //priority
    // const uint8_t addrH = (uint8_t) (address >> 8);
    trg[7] = (uint8_t) adr; //datasrc
    trg[8] = (uint8_t)(adr >> 8);
        //счетчик
    for (uint8_t i = 0; i < 4; i++)
        trg[9+i] = (uint8_t)(cnt >> i*8);
        
    trg[13] = (uint8_t)(ovrflw );
    trg[14] = (uint8_t)(ovrflw >> 8);
    trg[15] = 0;
    trg[16] = 0;
    trg[17] = 0;
    trg[18] = 0;
    trg[19] = 0;
}
void exlnk_setEnder(uint8_t * trg)
{
    trg[0] = 5;
    trg[1] = 5;
    trg[2] = 5;
    trg[3] = 5;
}
