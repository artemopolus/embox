#ifndef EXLNK_CMD_H_
#define EXLNK_CMD_H_


#include "exlnk_options.h"

typedef struct exlnk_cmd_str{
	uint8_t id;
   uint8_t address;
   uint8_t value;
}exlnk_cmd_str_t;

extern void exlnk_setCmd(exlnk_cmd_str_t * trg, uint8_t address, uint8_t value);
extern uint8_t exlnk_CmdToArray(exlnk_cmd_str_t * src, uint8_t * data, uint16_t datalen);
extern uint8_t exlnk_getCmd(exlnk_cmd_str_t * trg, uint8_t * data, uint16_t datalen);

#endif
