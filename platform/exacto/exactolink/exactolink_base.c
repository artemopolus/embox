#include "exactolink/exactolink_base.h"

void exlnk_initSDpack(const uint8_t type, const uint16_t cnt, const uint32_t refcnt, ExactoBufferExtended * buffer)
{
      // pshfrc_exbextu8(&ExDtStr_SD_buffer, 0x11);
            // pshfrc_exbextu8(&ExDtStr_SD_buffer, EXACTOLINK_SNS_XLXLGR);
            // pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data));
            //pshfrc_exbextu8(&ExDtStr_SD_buffer, (uint8_t)(cnt_data >> 8));
                //pshfrc_exbextu8(&ExDtStr_SD_buffer, value);
   pshfrc_exbextu8(buffer, EXACTOLINK_SDPACK_ID);
   pshfrc_exbextu8(buffer, type);
	

}
