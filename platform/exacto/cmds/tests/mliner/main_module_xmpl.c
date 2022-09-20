#include "mliner/mliner_sec.h"
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

typedef struct mliner_main_sender_buffer{
    mliner_cmd_info_t packs[MLINER_MAIN_SENDER_BUFFER_PACKSCNT_MAX];
    uint8_t cnt;
}mliner_main_sender_buffer_t;

static mliner_main_sender_buffer_t UploadBuffer = {0};
static mliner_main_sender_buffer_t TransmitBuffer = {0};

static uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;


static uint8_t Addresses[] = {7,16};
static exlnk_set_header_str_t SendBuffer[ECTM_SEC_COUNT];
static exlnk_get_header_str_t GettBuffer;

static uint8_t TmpBuffer[100];

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

static void prepareTransmit(uint8_t value)
{
    exlnk_set_header_str_t * trg = NULL;

    for(int i = 0; i < ECTM_SEC_COUNT; i++)
    {
        if(SendBuffer[i].adr == value)
            trg = &SendBuffer[i];
    }
    if(trg == NULL)
        return;
    exlnk_cmd_str_t out;
    if(value == 7)
    {
        exlnk_setCmd(&out, 65, 112);
        exlnk_uploadCmdHeader(trg, &out);
    }
    else
    {
        exlnk_setCmd(&out, 55, 86);
        exlnk_CmdToArray(&out, TmpBuffer, 100);
        exlnk_uploadHeader(trg, TmpBuffer, sizeof(exlnk_cmd_str_t));
    }
    
    exmliner_CmdInfoInit(&UploadBuffer.packs[UploadBuffer.cnt++], out.id, out.mnum);

    exlnk_closeHeader(trg);

    
    ECTM_SendData_Counter++;

}


static void sending(uint8_t value)
{
	for (int i = 0; i < ECTM_SEC_COUNT; i++)
	{
		if(SendBuffer[i].is_closed)
		{
			exlnk_initHeader(&SendBuffer[i], &ECTM_TransmitBuffer[i/2*EXACTO_BUFFER_UINT8_SZ]);
			exlnk_fillHeader(&SendBuffer[i], Addresses[i], EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);
		}
	}

	exlnk_getHeader(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer);
	uint16_t len = exlnk_isEmptyGetHeader(&GettBuffer);
	printf("[%5d][%5d] => ", ECTM_SendData_Counter, value);
	while(len > 0)
	{
		exlnk_cmd_str_t in;
		exlnk_cmdack_str_t ack;
		if(exlnk_getCmd(&in, &GettBuffer.data[GettBuffer.datapt], GettBuffer.datalen))
		{
			printf("cmd [ adr: %3d val: %3d ]", in.reg, in.value);
			GettBuffer.datapt += sizeof( exlnk_cmd_str_t);
			for (int i = 0; i< ECTM_SEC_COUNT; i++)
			{
				if(SendBuffer[i].adr == GettBuffer.adr)
				{
					exlnk_cmdack_str_t ack1;
					exlnk_setCmdAck(&ack1, in.id, in.mnum, in.reg);
					exlnk_CmdAckToArray(&ack1, TmpBuffer, 100);
					exlnk_uploadHeader(&SendBuffer[i], TmpBuffer, sizeof(exlnk_cmdack_str_t));
					printf("ACK [ adr: %3d val: %3d ]", ack1.reg, ack1.mnum);
				}
			}
		}
		else if(exlnk_getCmdAck(&ack, &GettBuffer.data[GettBuffer.datapt], GettBuffer.datalen))
		{
			// printf("ACK [ adr: %3d val: %3d ]", ack.reg, ack.mnum);
			for(int i =0 ; i < TransmitBuffer.cnt; i++)
			{
					if(TransmitBuffer.packs[i].mnum == ack.mnum)
						TransmitBuffer.packs[i].ack = 1;
			}
			GettBuffer.datapt += sizeof( exlnk_cmdack_str_t);
		}
		else
			break;
		len = exlnk_isEmptyGetHeader(&GettBuffer);
	}
	printf("\n");
	prepareTransmit(value);
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
   
	
	uint8_t i = 0;

	while (1)
	{
		while(!EnableUpdate);

		sending(AddressSendOrder[i++]);
		if(i >= AddressCount)
			i = 0;
		
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