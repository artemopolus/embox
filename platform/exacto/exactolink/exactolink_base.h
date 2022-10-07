#ifndef EXACTO_EXACTOLINK_EXACTOLINK_BASE_H
#define EXACTO_EXACTOLINK_EXACTOLINK_BASE_H

#include "commander/exacto_buffer.h"

#define EXACTOLINK_SDPACK_ID 0x11

#define EXACTOLINK_MLINE_SNSPACK_ID 0x17

extern uint8_t exlnk_pushSnsPack( const uint8_t sns_id, uint8_t * data, const uint16_t datacount,  ExactoBufferUint8Type * buffer);
extern void exlnk_initSDpack(const uint8_t type, const uint16_t datalen, const uint32_t refcnt, ExactoBufferExtended * buffer);
extern uint8_t exlnk_pushtoSDpack(const uint8_t value, ExactoBufferExtended * buffer);

extern uint16_t exlnk_getSDpack(uint8_t * type, uint16_t * datalen, uint32_t * refcnt, ExactoBufferExtended * buffer);
extern uint8_t exlnk_readsnsSDpack(const uint8_t type, uint8_t * sns_type, int16_t * dst, ExactoBufferExtended * buffer );
extern void exlnk_cv_Uint8_Uint16(uint8_t * src, uint16_t * dst);
extern void exlnk_cv_Uint8_Int16(uint8_t * src, int16_t * dst);
extern void exlnk_cv_Uint8_Uint32(uint8_t * src, uint32_t * dst);
extern void exlnk_cv_Int16_Uint8(const int16_t src, uint8_t * dst);
extern void exlnk_cv_Uint32_Uint8(const uint32_t src, uint8_t * dst);
extern void exlnk_cv_Uint16_Uint8(const uint16_t src, uint8_t * dst);
extern void exlnk_cv_Uint8_Uint16(uint8_t * src, uint16_t * dst);
#endif