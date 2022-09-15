#ifndef EXACTO_MLINER_MLINER_DEV_H_
#define EXACTO_MLINER_MLINER_DEV_H_

#include <stdint.h>
#include <unistd.h>
#include "exlnk_Cmd.h"

#define MLINER_SEC_MSG_SIZE EXACTO_BUFFER_UINT8_SZ
#define MLINER_SEC_MODULE_ADDRESS	7 


extern void exmliner_Init(uint16_t address);
extern void exmliner_Upload(void * data, size_t len, uint8_t id);
extern void exmliner_Update();
extern void exmliner_setCmdAction(int(*cmdaction)(exlnk_cmd_str_t * out));
extern void exmliner_setCmdAckAction(int(*cmdackaction)(exlnk_cmdack_str_t * out));
extern void exmliner_setResetAction(int(*resetaction)());
#endif //EXACTO_MLINER_MLINER_DEV_H_