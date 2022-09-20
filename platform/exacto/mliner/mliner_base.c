#include "mliner_sec.h"

#include "commander/exacto_buffer.h"
#include "commander/exacto_services.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"


#include "spi/spi_mline_sec.h"

#include <string.h>

#include "mliner/mliner.h"

static uint8_t NeedToSend = 0;


#define MLINER_MAIN

#define MLINER_SEC_PACKCNT_MAX 10

typedef struct mliner_sec_out_dev
{
	exlnk_set_header_str_t buffer;
	uint32_t counter;
	uint8_t dma[MLINER_SEC_MSG_SIZE];
	ExactoBufferUint8Type store;
	mliner_cmd_info_t uplpacks[MLINER_SEC_PACKCNT_MAX];
	uint8_t uplpacks_cnt;
	mliner_cmd_info_t outpacks[MLINER_SEC_PACKCNT_MAX];
	uint8_t outpacks_cnt;
}mliner_sec_out_dev_t;


typedef struct mliner_sec_in_dev
{
	exlnk_get_header_str_t buffer;
	uint32_t counter;
	uint8_t dma[MLINER_SEC_MSG_SIZE];
	ExactoBufferUint8Type store;
	uint8_t cmdaction_on;
	int(*cmdaction)(exlnk_cmd_str_t * out);
	uint8_t cmdackaction_on;
	int(*cmdackaction)(exlnk_cmdack_str_t * out);
	uint8_t onreset_on;
	int(*onreset)();
	uint8_t repeataction_on;
	int(*repeataction)(uint8_t id, uint32_t mnum);
	uint8_t erroraction_on;
	int(*erroraction)(int error);
}mliner_sec_in_dev_t;


static mliner_sec_out_dev_t Transmit ={0};
static mliner_sec_in_dev_t Receive = {0};
static uint8_t TmpBuffer[100] = {0};

void exmliner_Init(uint16_t address)
{
	setini_exbu8(&Receive.store);
	setini_exbu8(&Transmit.store);
	setRxBuffSpiDevSec(&Receive.store);
	setTxBuffSpiDevSec(&Transmit.store);
	Transmit.counter = 0;
	Receive.counter = 0;
	NeedToSend = 0;


	exlnk_initHeader(&Transmit.buffer, Transmit.dma);
	exlnk_fillHeader(&Transmit.buffer, MLINER_SEC_MODULE_ADDRESS, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, Transmit.counter, 0);
}
void exmliner_Upload(void * data, size_t len, uint8_t id)
{
	if (id == EXLNK_DATA_ID_CMD)
	{
		exlnk_cmd_str_t * cmd = (exlnk_cmd_str_t*)data;
		exlnk_CmdToArray(cmd, TmpBuffer, 100);
		if(Transmit.uplpacks_cnt + 1 < MLINER_SEC_PACKCNT_MAX)
		{
			Transmit.uplpacks[Transmit.uplpacks_cnt].id = cmd->id;
			Transmit.uplpacks[Transmit.uplpacks_cnt].mnum = cmd->mnum;
			Transmit.uplpacks[Transmit.uplpacks_cnt++].ack = 0;
		}
	}
	else if (id == EXLNK_DATA_ID_CMDACK)
		exlnk_CmdAckToArray((exlnk_cmdack_str_t*)data, TmpBuffer, 100);
	else
		return;
	exlnk_uploadHeader(&Transmit.buffer, TmpBuffer, len);
}
void exmliner_Update()
{
	if(getReceivedDataSpiDevSec(Receive.dma, MLINER_SEC_MSG_SIZE))
	{
		memset(&Receive.buffer, 0, sizeof(Receive.buffer));
		if(exlnk_getHeader(Receive.dma, MLINER_SEC_MSG_SIZE, &Receive.buffer))
		{
			Receive.counter = 0;
		}
		else
		{
			Receive.counter++;
		}
	}
	//process
	uint16_t len = exlnk_isEmptyGetHeader(&Receive.buffer);
	while(len > 0)
	{
		exlnk_cmd_str_t in;
		exlnk_cmdack_str_t ack;
			
		if(exlnk_getCmd(&in, &Receive.buffer.data[Receive.buffer.datapt], Receive.buffer.datalen))
		{
			Receive.buffer.datapt += sizeof( exlnk_cmd_str_t);
			//process
			if(Receive.buffer.adr == MLINER_SEC_MODULE_ADDRESS)
			{
				if(Receive.cmdaction_on)
					Receive.cmdaction(&in);
				exlnk_cmdack_str_t ack1;
				exlnk_setCmdAck(&ack1, in.id, in.mnum, in.reg);
				exmliner_Upload(&ack1, sizeof(exlnk_cmdack_str_t), EXLNK_DATA_ID_CMDACK);
				NeedToSend = 1;
			}
		}
		else if(exlnk_getCmdAck(&ack, &Receive.buffer.data[Receive.buffer.datapt], Receive.buffer.datalen))
		{
			Receive.buffer.datapt += sizeof( exlnk_cmdack_str_t);
			if(Receive.buffer.adr == MLINER_SEC_MODULE_ADDRESS)
			{
				if(Receive.cmdackaction_on)
					Receive.cmdackaction(&ack);


				for(int i = 0; i < Transmit.outpacks_cnt; i++)
				{
					if(Transmit.outpacks[i].mnum == ack.mnum)
						Transmit.outpacks[i].ack = 1;
				}

			}
		}
		else
			break;
		len = exlnk_isEmptyGetHeader(&Receive.buffer);
	}
	//switch
#ifdef MLINER_MAIN
	if(1)
#elif
	if(NeedToSend)
#endif
	{
		uint8_t to_repeat = 1;

		if(Transmit.outpacks_cnt)
		{
			for(int i = 0; i < Transmit.outpacks_cnt; i++)
			{
				if(Transmit.outpacks[i].ack)
				{
					to_repeat = 0;
					break;
				}
			}
		}
		else
			to_repeat = 0;
		

		if(!to_repeat)//not send
		{
			//transmit
			exlnk_closeHeader(&Transmit.buffer);
			for (int i = 0; i < Transmit.uplpacks_cnt; i++)
			{
				Transmit.outpacks[i] = Transmit.uplpacks[i];
				Transmit.outpacks_cnt++;
			}
			Transmit.uplpacks_cnt = 0;
			

			setTransmitDataSpiDevSec(Transmit.buffer.data, Transmit.buffer.pt_data);

			if(!transmitSpiDevSec()&& Receive.erroraction_on)
				Receive.erroraction(1);
			NeedToSend = 0;
			Transmit.counter++;
			exlnk_clearSetHeader(&Transmit.buffer);
			//new
			exlnk_initHeader(&Transmit.buffer, Transmit.dma);
			exlnk_fillHeader(&Transmit.buffer, MLINER_SEC_MODULE_ADDRESS, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, Transmit.counter, 0);
		}
		else
		{
			if(!repeatTransmitSpiDevSec()&&Receive.erroraction_on)
				Receive.erroraction(2);
			NeedToSend = 0;
			for(int i = 0; i < Transmit.outpacks_cnt; i++)
				Transmit.outpacks[i].ack = 0;
			if(Receive.repeataction_on)
			{
				for(int i = 0; i < Transmit.outpacks_cnt; i++)
					Receive.repeataction(Transmit.outpacks[i].id, Transmit.outpacks[i].mnum);
			}
		}
	}
	else
	{
		//receive
		receiveSpiDevSec();
	}

	//check
#ifndef MLINER_MAIN
	if(Receive.counter >= 999)
	{
		if(Receive.onreset_on)
			Receive.onreset();
		ipl_disable();
		Receive.counter = 0;
		ipl_enable();
		disableSpiDevSec();
	}
#endif
}
void exmliner_setCmdAction(int(*cmdaction)(exlnk_cmd_str_t * out))
{
	Receive.cmdaction = cmdaction;
	Receive.cmdaction_on = 1;
}
void exmliner_setCmdAckAction(int(*cmdackaction)(exlnk_cmdack_str_t * out))
{
	Receive.cmdackaction = cmdackaction;
	Receive.cmdackaction_on = 1;
}
void exmliner_setResetAction(int(*resetaction)())
{
	Receive.onreset = resetaction;
	Receive.onreset_on = 1;
}

void exmliner_setRepeatAction(int(*repeataction)(uint8_t id, uint32_t mnum))
{
	Receive.repeataction = repeataction;
	Receive.repeataction_on = 1;
}
void exmliner_setErrorAction(int(*erroraction)(int id))
{
	Receive.erroraction = erroraction;
	Receive.erroraction_on = 1;
}
