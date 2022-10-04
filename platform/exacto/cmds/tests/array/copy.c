#include "ex_utils.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CMDS_TESTS_ARRAY_COPY_SZ 1024
static exutils_data_t TagTimer;

int main(int argc, char *argv[]) 
{
	ex_dwt_cyccnt_reset();
	exutils_init(&TagTimer);

	uint32_t duration = 0;

	uint8_t src[CMDS_TESTS_ARRAY_COPY_SZ] = {5};
	uint8_t trg[CMDS_TESTS_ARRAY_COPY_SZ] = {3};
	
	exutils_updt(&TagTimer);
	for(int i = 0; i < CMDS_TESTS_ARRAY_COPY_SZ; i++)
	{
		trg[i] = src[i];
	}
	exutils_updt(&TagTimer);
	duration = TagTimer.result;
	printf("Standart: %d\n", duration);

	uint8_t * trg_pt = &trg[0];
	uint8_t * src_pt = &src[0];
	exutils_updt(&TagTimer);
	for(int i = 0; i < CMDS_TESTS_ARRAY_COPY_SZ; i++)
	{
		*(trg_pt++) = *(src_pt++);
	}
	exutils_updt(&TagTimer);
	duration = TagTimer.result;
	printf("Pointer: %d\n", duration);

	uint16_t * trg_pt16 = (uint16_t *)&trg[0];
	uint16_t * src_pt16 = (uint16_t*)&src[0];
	int len = CMDS_TESTS_ARRAY_COPY_SZ/2;
	exutils_updt(&TagTimer);
	for(int i = 0; i < len ; i++)
	{
		*(trg_pt16++) = *(src_pt16++);
	}
	exutils_updt(&TagTimer);
	duration = TagTimer.result;
	printf("Pointer16: %d\n", duration);

	exutils_updt(&TagTimer);
	memcpy(src, trg, CMDS_TESTS_ARRAY_COPY_SZ);	
	exutils_updt(&TagTimer);
	duration = TagTimer.result;
	printf("Memcpy: %d\n", duration);

	return 0;
}