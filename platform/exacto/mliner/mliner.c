#include "mliner/mliner.h"

void exmliner_CmdInfoInit(mliner_cmd_info_t * trg, uint8_t id, uint32_t mnum)
{
	trg->id = id;
	trg->mnum = mnum;
	trg->ack = 0;
}

