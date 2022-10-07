#include "mliner/mliner.h"

void exmliner_CmdInfoInit(mliner_cmd_info_t * trg, uint8_t id, uint32_t mnum)
{
	trg->id = id;
	trg->mnum = mnum;
	trg->ack = 0;
}
void exmliner_DevInfoInit(mliner_dev_info_t * trg)
{
	trg->count = 0;
	trg->dblcnt = 0;
	trg->ovrflw = 0;
}

