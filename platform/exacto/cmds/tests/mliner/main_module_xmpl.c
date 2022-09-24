#include "mliner/mliner_main.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>
#include "commander/exacto_buffer.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"
#include "exlnk_Cmd.h"
#include "mliner/mliner.h"


#define MLINER_MAIN_SENDER_BUFFER_PACKSCNT_MAX 10
#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
#define ECTM_SEC_COUNT 2



// uint8_t AddressSendOrder[] = {7, 7, 7, 7, 7, 0,0,0,0,0, 16, 16, 16, 16, 16, 0,0,0,0,0};
// uint8_t AddressSendOrder[] = {7, 0,0, 16, 0,0};
static uint8_t AddressSendOrder[] = {7, 16, 7, 16, 7, 16};
static uint8_t AddressCount = 6;






static uint16_t TIM_Counter = 0;
static uint16_t SendCounter = 0;

static int PointToTim;
static uint8_t EnableUpdate = 0;
static uint8_t NeedToPrint = 0;


static int run_Tim_Lthread(struct  lthread * self)
{
	exse_ack(&ExTimServices[PointToTim]);
	if(TIM_Counter < 10000)
		TIM_Counter++;
	else
		TIM_Counter = 0;

	if(NeedToPrint == 0)
	{
		if(! (TIM_Counter % 200))
			NeedToPrint = 1;
	}
	if(! (TIM_Counter % 200))
		EnableUpdate = 1;
	return 0;
}
static int onCmdEventHandler(exlnk_cmd_str_t * cmd)
{
	printf("in:[reg: %3d val: %3d]\n", cmd->reg, cmd->value);
	// cmd->value += 3;
	// exmliner_Upload(cmd, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD);
	// SendCounter++;
	return 0;
}
static int onCmdAckEventHandler(exlnk_cmdack_str_t * cmd)
{
	printf("ack:[mnum: %5d reg: %5d]\n", cmd->mnum, cmd->reg);
	return 0;
}
static int onResetEventHandler()
{
	printf("Try reset Mline\n");
	return 0;
}
static int onRepeatEventHandler(uint8_t id, uint32_t mnum)
{
	printf("repeat: [%d %d ]\n", id, mnum);
	return 0;
}
static int onErrorEventHandler(int id)
{
	if(id == 1)
	{
		printf("Transmit failed\n");
	}
	else if (id == 2)
	{
		printf("Repeat failed\n");
	}
	return 0 ;
}


static void sending(uint8_t value)
{
	exlnk_cmd_str_t cmd;
   if(value == 7)
	{
      exlnk_setCmd(&cmd, 65, 112);
	}
	else if(value == 16)
	{
      exlnk_setCmd(&cmd, 55, 86);

	}
	exmliner_Upload(&cmd, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD, value);
	printf("\n");
}
int main(int argc, char *argv[]) 
{
	exmliner_setCmdAction(onCmdEventHandler);
	exmliner_setResetAction(onResetEventHandler);
	exmliner_setCmdAckAction(onCmdAckEventHandler);
	exmliner_setRepeatAction(onRepeatEventHandler);
	exmliner_setErrorAction(onErrorEventHandler);

	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0);
   
	
 	uint8_t index = 0;
	while (1)
	{
		while(!EnableUpdate);

		uint16_t trg_adr = AddressSendOrder[index++];

		sending(trg_adr);
		exmliner_Update(trg_adr);

		if(index >= AddressCount)
			index = 0;
		
		if(NeedToPrint)
		{
			printf("tim[%8d]send[%5d][%3d]\n", TIM_Counter,SendCounter, trg_adr);
			NeedToPrint = 0;
		}
		EnableUpdate = 0;
		// sleep(1);
	}
	
	return 1;
}