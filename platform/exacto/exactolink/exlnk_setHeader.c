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
void exlnk_initHeader(exlnk_set_header_str_t * trg, uint8_t * buffer)
{
    trg->pt_data = 0;
    trg->data = buffer;
}
void exlnk_fillHeader(exlnk_set_header_str_t * trg, uint16_t adr, uint8_t type_msg, uint8_t type_pack, uint16_t len, uint32_t cnt, uint16_t ovrflw)
{
    exlnk_setHeader(adr, trg->data, type_msg, type_pack, len, cnt, ovrflw);
    trg->pt_data = type_pack;
    trg->adr = adr;
    trg->cnt = cnt;
    trg->type_msg = type_msg;
    trg->type_pack = type_pack;
    trg->len = len;
    trg->ovrflw = ovrflw;
}
void exlnk_uploadHeader(exlnk_set_header_str_t * trg, uint8_t * datasrc, uint16_t datalen)
{
    for (uint16_t i = 0; i < datalen; i++)
    {
        trg->data[trg->pt_data++] = datasrc[i];
    }
}
void exlnk_closeHeader(exlnk_set_header_str_t * trg)
{
    trg->data[trg->pt_data++] = 5;
    trg->data[trg->pt_data++] = 5;
    trg->data[trg->pt_data++] = 5;
    trg->data[trg->pt_data++] = 5;
    uint16_t length = trg->pt_data;
    trg->data[1] = (uint8_t) (length);
    trg->data[2] = (uint8_t) (length >> 8);
    trg->len = length;
}

