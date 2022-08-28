#include "exlnk_Cmd.h"
#include <string.h>

void exlnk_setCmd(exlnk_cmd_str_t * trg, uint8_t address, uint8_t value)
{
	trg->id = EXLNK_DATA_ID_CMD;
	trg->address = address;
	trg->value = value;
}
uint8_t exlnk_CmdToArray(exlnk_cmd_str_t * src, uint8_t * data, uint16_t datalen)
{
	if(datalen < sizeof(exlnk_cmd_str_t))
		return 0;
	memcpy(data, src, sizeof( exlnk_cmd_str_t));
	return 1;
}
uint8_t exlnk_getCmd(exlnk_cmd_str_t * trg, uint8_t * data, uint16_t datalen)
{
	if(data[0] != EXLNK_DATA_ID_CMD && datalen < sizeof(exlnk_cmd_str_t))
		return 0;
	memcpy(trg, data, sizeof( exlnk_cmd_str_t));
	return 1;
}
