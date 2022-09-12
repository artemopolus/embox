#include "exlnk_getHeader.h"


uint8_t exlnk_getHeader( uint8_t * trg, uint16_t len, exlnk_get_header_str_t * out)
{
    if(trg[0] != 17)
        return 0;
    if(len < 19)
        return 0;
    out->len = (uint16_t) trg[1];
    out->len += (uint16_t) (trg[2] << 8);
	if(out->len > len)
		return 0;
    out->type_msg = trg[3];
    out->type_pack = trg[4];
    if (out->type_pack != EXLNK_PACK_SIMPLE && trg[out->len] != 5)
        return 0;
    out->adr = (uint16_t) trg[7];
    out->adr += (uint16_t) (trg[8] << 8);
    out->cnt = 0;
    for(uint8_t i = 0; i < 4; i++)
        out->cnt += (uint32_t)(trg[9+i] << i*8);
    out->ovrflw = (uint16_t) (trg[13]);
    out->ovrflw += (uint16_t) (trg[14] << 8);
    out->data = &trg[EXLNK_PACK_SIMPLE];
    out->datapt = 0;
    out->datalen = out->len - EXLNK_PACK_SIMPLE - 4;
    return 1;
}
uint8_t exlnk_getHeaderPackData(exlnk_get_header_str_t * trg, uint8_t * value)
{
    if(trg->datapt < trg->datalen)
        *value = trg->data[trg->datapt++];
    else
        return 0;
    return 1;
}
uint16_t exlnk_isEmptyGetHeader(exlnk_get_header_str_t * trg)
{
    return (trg->len - trg->datapt - 4);
}
