#include "exactolink/exactolink_base.h"
#include <stdio.h>
uint8_t exlnk_pushSnsPack( const uint8_t sns_id, uint8_t * data, const uint16_t datacount,  ExactoBufferUint8Type * buffer)
{
	if (checkSpace_exbu8(buffer,(datacount + 2)))
	{
		pshsft_exbu8(buffer, EXACTOLINK_MLINE_SNSPACK_ID);
		pshsft_exbu8(buffer, sns_id);
		writetoSpace_exbu8(buffer, data, datacount);
		return 1;
	}
	return 0;
}
void exlnk_initSDpack(const uint8_t type, const uint16_t cnt, const uint32_t refcnt, ExactoBufferExtended * buffer)
{
      // pshfrc_exbextu8(&ExDtStr_SD_buffer, 0x11);
            // pshfrc_exbextu8(&ExDtStr_SD_buffer, EXACTOLINK_SNS_XLXLGR);
            // pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data));
            //pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data >> 8));
                //pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
   pshfrc_exbextu8(buffer, EXACTOLINK_SDPACK_ID);
   pshfrc_exbextu8(buffer, type);
	pshsft_exbextu8(buffer, (uint8_t) (cnt));
	pshsft_exbextu8(buffer, (uint8_t) (cnt >> 8));
	pshsft_exbextu8(buffer, (uint8_t) (refcnt));
	pshsft_exbextu8(buffer, (uint8_t) (refcnt >> 8));
	pshsft_exbextu8(buffer, (uint8_t) (refcnt >> 16));
	pshsft_exbextu8(buffer, (uint8_t) (refcnt >> 24));
}

uint16_t exlnk_getSDpack(uint8_t * type, uint16_t * datalen, uint32_t * refcnt, ExactoBufferExtended * buffer)
{
	uint8_t value, is_pck = 0;
	uint8_t arr[4] = {0}; 
	uint16_t index = 0;
	while(grbfst_exbextu8(buffer, &value))
	{
		index++;
		if (value == EXACTOLINK_SDPACK_ID)
		{
			is_pck = 1;
			break;
		}
	}
	if (!is_pck)
		return 0;
	grbfst_exbextu8(buffer, &value);
	*type = value;
	for(int i = 0; (i < 2); i++)
	{
		if(!grbfst_exbextu8(buffer, &arr[i]))
			return 0;
	}
	// printf("%d %d", arr[0], arr[1]);
	exlnk_cv_Uint8_Uint16(arr, datalen);
	// printf("dt=%d", *datalen);
	for(int i = 0; (i < 4); i++)
	{
		if(!grbfst_exbextu8(buffer, &arr[i]))
			return 0;
	}
	exlnk_cv_Uint8_Uint32(arr, refcnt);
	// printf("dt=%d", *refcnt);
	return index += 7;
}
uint8_t exlnk_readsnsSDpack(const uint8_t type, uint8_t * sns_type, int16_t * dst, ExactoBufferExtended * buffer )
{
	uint8_t is_pck = 0, datalen = 0;
	uint8_t arr[2]; 
	// printf("dddddone\n");
	while (grbfst_exbextu8(buffer, arr))
	{
		if (arr[0] == EXACTOLINK_MLINE_SNSPACK_ID)
		{
			is_pck = 1;
			break;
		}
	}
	if (!is_pck)
		return 0;
	// printf("dddddone\n");
	if (!grbfst_exbextu8(buffer, sns_type))
		return 0;
	if (*sns_type == 1)
		datalen = 3;
	else if (*sns_type == 2)
		datalen = 6;
	// printf("dddddone\n");
	for (uint8_t i = 0; i < datalen; i++)
	{
		for (uint8_t j = 0; (j < 2); j++)
		{
			if (!grbfst_exbextu8(buffer, &arr[j]))
				return 0;
			if (j == 1)
				exlnk_cv_Uint8_Int16(arr, &dst[i]);	
		}
	}
	return datalen;
}
uint8_t exlnk_pushtoSDpack(const uint8_t value, ExactoBufferExtended * buffer)
{
	return pshsft_exbextu8(buffer, value);
}
void exlnk_cv_Uint8_Uint16(uint8_t * src, uint16_t * dst)
{
    uint16_t first = (uint16_t) src[1];
    *dst = (first << 8) + (uint16_t)src[0];
}
void exlnk_cv_Uint8_Int16(uint8_t * src, int16_t * dst)
{
    int16_t first = (int16_t) src[1];
    *dst = (first << 8) + (int16_t)src[0];
}
void exlnk_cv_Uint8_Uint32(uint8_t * src, uint32_t * dst)
{
    *dst = 0;
    for (uint16_t i = 0; i < 4; i++)
        *dst += (uint64_t) (src[i] << i*8);
}
void exlnk_cv_Int16_Uint8(const int16_t src, uint8_t * dst)
{
	dst[0] = (uint8_t) (src);
   dst[1] = (uint8_t) (src >> 8);
}
void exlnk_cv_Uint32_Uint8(const uint32_t src, uint8_t * dst)
{
   for (uint16_t i = 0; i < 4; i++)
      dst[i] = (uint8_t)(src >> 8*i);
}
void exlnk_cv_Uint16_Uint8(const uint16_t src, uint8_t * dst)
{
	dst[0] = (uint8_t) (src);
	dst[1] = (uint8_t) (src >> 8);
}
