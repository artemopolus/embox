#include "mliner/mliner_sec.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>

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
	EnableUpdate = 1;
	return 0;
}
static int onCmdEventHandler(exlnk_cmd_str_t * cmd)
{
	printf("in:[reg: %3d val: %3d]\n", cmd->reg, cmd->value);
	cmd->value += 3;
	exmliner_Upload(cmd, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD);
	SendCounter++;
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
	while (1)
	{
		while(!EnableUpdate);
		exmliner_Update();
		if(NeedToPrint)
		{
			printf("tim[%8d]send[%5d]\n", TIM_Counter,SendCounter);
			NeedToPrint = 0;
		}
		EnableUpdate = 0;
		// sleep(1);
	}
	
	return 1;
}