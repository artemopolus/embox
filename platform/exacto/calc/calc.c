#include "calc/calc.h"


#include <errno.h>
#include <embox/unit.h>

#define EXCC_CALC_BUFFER_SZ 512

typedef struct excc_calc_dev{
	uint16_t(*get)(uint8_t * data, const uint16_t datalen);
	uint8_t data[EXCC_CALC_BUFFER_SZ];
}excc_calc_dev_t;

excc_calc_dev_t CalcDev = {0};

EMBOX_UNIT_INIT(excc_init);
int excc_init()
{
	return 0;
}

uint8_t excc_setGetter(uint16_t(*receive)(uint8_t * data, const uint16_t datalen))
{
	CalcDev.get = receive;
	return 1;
}

uint8_t excc_exeProcess()
{
	uint16_t len = CalcDev.get(CalcDev.data, EXCC_CALC_BUFFER_SZ);
	if(len > 0)
	{
		//process
		return 1;
	}
	return 0;
}

