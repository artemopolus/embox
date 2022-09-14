#ifndef EXACTO_MLINER_MLINER_DEV_H_
#define EXACTO_MLINER_MLINER_DEV_H_

#include <stdint.h>
typedef struct exmliner_dev
{
	uint8_t is_enabled;
	int (*onUpdate)(uint8_t * data, uint16_t datalen, uint8_t id);
}exmliner_dev_t;

extern void exmliner_cr(exmliner_dev_t * trg);
extern void exmliner_init(exmliner_dev_t * trg, int (*onUpdate)(uint8_t * data, uint16_t datalen, uint8_t id));
extern void exmliner_run(exmliner_dev_t * trg, uint8_t * data, uint16_t datalen, uint8_t id);

#endif //EXACTO_MLINER_MLINER_DEV_H_