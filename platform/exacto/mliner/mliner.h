#ifndef EXACTO_MLINER_MLINER_H_
#define EXACTO_MLINER_MLINER_H_

#include "stdint.h"

typedef struct mliner_cmd_info
{
	uint8_t id;
	uint32_t mnum;
	uint8_t ack;
}mliner_cmd_info_t;

extern void exmliner_CmdInfoInit(mliner_cmd_info_t * trg, uint8_t id, uint32_t mnum);

#endif //EXACTO_MLINER_MLINER_H_