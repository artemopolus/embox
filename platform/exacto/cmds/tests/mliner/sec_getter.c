#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/printk.h>


#include "commander/exacto_data_storage.h"
#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"
#include "exlnk_Cmd.h"

#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;

exlnk_set_header_str_t SendBuffer;
exlnk_get_header_str_t GettBuffer;

uint8_t TmpBuffer[100];

static void sending(int value)
{
	receiveExactoDataStorage();

	printf("iteration[%d]\n", ECTM_SendData_Counter);
	exds_getData(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, 0); 


	exlnk_getHeader(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer);

	if(GettBuffer.adr == 7)
	{
		exlnk_cmd_str_t in;
		exlnk_getCmd(&in, &GettBuffer.data[GettBuffer.datapt], GettBuffer.datalen);

    	printf(" Cnt: %d\nin:\nadr: %d\nval: \n", ECTM_SendData_Counter, in.address, in.value);

		exlnk_initHeader(&SendBuffer, ECTM_TransmitBuffer);
		exlnk_fillHeader(&SendBuffer, 7, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);

		exlnk_CmdToArray(&in, TmpBuffer, 100);
		exlnk_uploadHeader(&SendBuffer, TmpBuffer, sizeof(exlnk_cmd_str_t));

		exlnk_closeHeader(&SendBuffer);

		ECTM_SendData_Counter++;
		exds_setData(SendBuffer.data, SendBuffer.pt_data, EX_THR_CTRL_OK);

		transmitExactoDataStorage();
	}
}
static void init()
{
    ECTM_SendData_Counter = 0;
}
int main(int argc, char *argv[]) 
{

    init();
    while(1)
    {
		sending(0);
		sleep(2);
    }
    return 1;
}

