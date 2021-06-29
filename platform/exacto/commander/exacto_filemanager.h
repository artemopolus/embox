#ifndef EXACTO_FILEMANAGER_H
#define EXACTO_FILEMANAGER_H
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "exacto_buffer.h"
#include "exacto_data_storage.h"
//переменные
// extern uint32_t EFM_
//функции
extern uint8_t ex_writeToLogChar(char * info);
extern uint8_t ex_saveExBufToFile( ExactoBufferUint8Type * buffer );
extern uint8_t ex_pshExBufToSD(  );
extern uint8_t ex_saveToFile(uint8_t * data, uint16_t datalen);
extern uint8_t ex_saveToLog(uint8_t * data, uint16_t datalen);
extern uint8_t initExactoFileManager(void);
#endif
