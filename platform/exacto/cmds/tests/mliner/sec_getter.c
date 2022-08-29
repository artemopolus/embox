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

#include "tim/tim.h"

#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
static uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;

static exlnk_set_header_str_t SendBuffer;
static exlnk_get_header_str_t GettBuffer;

static uint8_t TmpBuffer[100];

static uint8_t NeedToSend = 0;

// static struct lthread Tim_Lthread;
static int run_Tim_Lthread(struct  lthread * self)
{
	checking();
	return 0;
}

static void checking()
{
	if(!NeedToSend)
	{
		receiveExactoDataStorage();

		printf("iteration[%d]\n", ECTM_SendData_Counter);
		if(exds_getData(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, 0) > 0)
		{
			memset(&GettBuffer, 0, sizeof(GettBuffer));
			exlnk_getHeader(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer);
			if(GettBuffer.adr == 7)
			{
				NeedToSend = 1;
			}
		}
	}
}

static void sending()
{
	if(NeedToSend)
	{	
		exlnk_cmd_str_t in;
		exlnk_getCmd(&in, &GettBuffer.data[GettBuffer.datapt], GettBuffer.datalen);
		NeedToSend = 0;

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
 	ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);

	ex_setFreqHz(100);
}
int main(int argc, char *argv[]) 
{

    init();
    while(1)
    {
		sending();
		sleep(2);
    }
    return 1;
}

