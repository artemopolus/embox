#ifndef EXACTO_FILEMANAGER_H
#define EXACTO_FILEMANAGER_H
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
extern uint8_t ex_saveToFile(uint8_t * data, uint16_t datalen);
extern uint8_t ex_saveToLog(uint8_t * data, uint16_t datalen);
extern uint8_t initExactoFileManager(void);
#endif
