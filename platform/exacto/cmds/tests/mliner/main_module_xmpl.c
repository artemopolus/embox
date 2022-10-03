#include "mliner/mliner_main.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>
#include "commander/exacto_buffer.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"
#include "exlnk_Cmd.h"
#include "mliner/mliner.h"

#include "ex_utils.h"

#define MLINER_MAIN_SENDER_BUFFER_PACKSCNT_MAX 10
#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
#define ECTM_SEC_COUNT 2
#define TIM_1SEC_DIVIDER 200


// uint8_t AddressSendOrder[] = {7, 7, 7, 7, 7, 0,0,0,0,0, 16, 16, 16, 16, 16, 0,0,0,0,0};
// uint8_t AddressSendOrder[] = {7, 0,0, 16, 0,0};
static uint8_t AddressSendOrder[] = {7, 16, 7, 16, 7, 16};
static uint8_t AddressCount = 6;


static uint8_t index = 0;




static uint16_t TIM_Counter = 0;
static uint16_t SendCounter = 0;

static int PointToTim;
static uint8_t 
					EnableUpdate = 0,
					NeedToPrint = 0,
					ReceiveDone = 0
					;

#define MEASURE_TIME

#ifdef MEASURE_TIME
static exutils_data_t TagTimer;
static uint32_t 	
						TimerMlineDuration = 0,
						TimerMlineDurationAVR = 0,
						LoadInMlineDuration = 0,
						LoadInMlineDurationAVR = 0,
						UpdateMlineDuration = 0,
						UpdateMlineDurationAVR = 0,
						TransmitMlineDuration = 0,
						TransmitMlineDurationAVR = 0
						;
#endif

static int run_Tim_Lthread(struct  lthread * self)
{
	exse_ack(&ExTimServices[PointToTim]);
	if(TIM_Counter < 10000)
		TIM_Counter++;
	else
		TIM_Counter = 0;

	if(NeedToPrint == 0)
	{
		if(! (TIM_Counter % TIM_1SEC_DIVIDER))
			NeedToPrint = 1;
	}
	if(! (TIM_Counter % TIM_1SEC_DIVIDER))
		EnableUpdate = 1;
	return 0;
}
static int onCmdEventHandler(exlnk_cmd_str_t * cmd)
{
	printf("inCmd:[reg: %3d val: %3d]", cmd->reg, cmd->value);
	// cmd->value += 3;
	// exmliner_Upload(cmd, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD);
	// SendCounter++;
	return 0;
}
static int onCmdAckEventHandler(exlnk_cmdack_str_t * cmd)
{
	printf("inAck:[mnum: %5d reg: %5d]", cmd->mnum, cmd->reg);
	mliner_cmd_info_t * trg = NULL;
	uint8_t *trgcnt = NULL;
	exmliner_getSendPacks(trg, trgcnt, AddressSendOrder[index]);
	for(int i = 0;trg && trgcnt && i < *trgcnt; i++)
	{
		if(trg[i].mnum == cmd->mnum)
		{
			printf("Done");
			SendCounter++;
		}
		else
		{
			printf("Unkn");
		}
	}
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
	exutils_updt(&TagTimer);
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
	exutils_updt(&TagTimer);
	LoadInMlineDuration = TagTimer.result;
	// printf("TagUpl[%8d]", LoadInMlineDuration);
	printf("outCmd[%5d %3d %3d]", cmd.mnum, cmd.reg, cmd.value);
	exutils_updt(&TagTimer);
}
int main(int argc, char *argv[]) 
{
	exmliner_setCmdAction(onCmdEventHandler);
	exmliner_setResetAction(onResetEventHandler);
	exmliner_setCmdAckAction(onCmdAckEventHandler);
	exmliner_setRepeatAction(onRepeatEventHandler);
	exmliner_setErrorAction(onErrorEventHandler);

	ex_dwt_cyccnt_reset();
	exutils_init(&TagTimer);


	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0);
   
	
	while (1)
	{
		while(!EnableUpdate);

		exutils_updt(&TagTimer);
		TimerMlineDuration = TagTimer.result;
		uint16_t trg_adr = AddressSendOrder[index++];

		sending(trg_adr);
		exmliner_Update(trg_adr);
		exutils_updt(&TagTimer);
		UpdateMlineDuration = TagTimer.result;
		while (!ReceiveDone)
		{
			if(exmliner_getRxIRQ())
				ReceiveDone = 1;
		}
		exutils_updt(&TagTimer);
		TransmitMlineDuration = TagTimer.result;

		LoadInMlineDurationAVR += LoadInMlineDuration;
		UpdateMlineDurationAVR += UpdateMlineDuration;
		TransmitMlineDurationAVR += TransmitMlineDuration;
		TimerMlineDurationAVR += TimerMlineDuration;

		if(index >= AddressCount)
			index = 0;
		
		if(NeedToPrint)
		{
			// printf("tim[%8d]send[%5d][%3d]\n", TIM_Counter,SendCounter, trg_adr);
			LoadInMlineDurationAVR = LoadInMlineDurationAVR / TIM_1SEC_DIVIDER;
			UpdateMlineDurationAVR = UpdateMlineDurationAVR / TIM_1SEC_DIVIDER;
			TransmitMlineDurationAVR = TransmitMlineDurationAVR /TIM_1SEC_DIVIDER;
			TimerMlineDurationAVR = TimerMlineDurationAVR /TIM_1SEC_DIVIDER;
			printf("\n      TIM|  Send| Adr|    Load |  Update | Transmit| Duration| \n %8d| %5d| %3d| %8d| %8d| %8d| %d\n",TIM_Counter, SendCounter, trg_adr, LoadInMlineDurationAVR, UpdateMlineDurationAVR, TransmitMlineDurationAVR, TimerMlineDurationAVR);

			LoadInMlineDurationAVR = 0;
			UpdateMlineDurationAVR = 0;
			TransmitMlineDurationAVR = 0;
			TimerMlineDurationAVR = 0;
			NeedToPrint = 0;
		}
		EnableUpdate = 0;
		ReceiveDone = 0;

		exutils_updt(&TagTimer);
		
		// sleep(1);
	}
	
	return 1;
}