#include "mliner/mliner_dev.h"

void exmliner_cr(exmliner_dev_t * trg)
{
	trg->is_enabled = 0;
}
void exmliner_init(exmliner_dev_t * trg, int (*onUpdate)(uint8_t * data, uint16_t datalen, uint8_t id))
{
	trg->onUpdate = onUpdate;
	trg->is_enabled = 1;
}

void exmliner_run(exmliner_dev_t * trg, uint8_t * data, uint16_t datalen, uint8_t id)
{
	if(trg->is_enabled)
	{
		trg->onUpdate(data, datalen, id);
	}
}
