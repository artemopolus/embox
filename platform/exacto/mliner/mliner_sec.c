#include "mliner_sec.h"

#include "commander/exacto_buffer.h"
#include "commander/exacto_services.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"

#include "tim/tim.h"

#include "spi/spi_mline_sec.h"

#include <string.h>


static uint8_t NeedToSend = 0;

// static exlnk_set_header_str_t SendBuffer;
// static uint32_t ECTM_SendData_Counter = 0;
// static uint8_t ECTM_TransmitBuffer[MLINER_SEC_MSG_SIZE] = {0};

typedef struct mliner_cmd_info
{
	uint8_t id;
	uint32_t mnum;
	uint8_t ack;
	uint8_t is_send;
}mliner_cmd_info_t;

static mliner_cmd_info_t SendCmd = {0};
static mliner_cmd_info_t SendCmdBuff = {0};

typedef struct mliner_sec_dev
{
	exlnk_set_header_str_t buffer;
	uint32_t counter;
	uint8_t dma[MLINER_SEC_MSG_SIZE];
	ExactoBufferUint8Type store;
}mliner_sec_dev_t;


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
}mliner_sec_in_dev_t;


static mliner_sec_dev_t Transmit ={0};
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
		if((SendCmd.id && SendCmd.ack )||!SendCmd.id)
		{
			SendCmd.id = cmd->id;
			SendCmd.mnum = cmd->mnum;
			SendCmd.ack = 0;
			SendCmd.is_send = 0;
		}
		else if(SendCmd.id && !SendCmd.ack && !SendCmdBuff.id)
		{
			SendCmdBuff.id = cmd->id;
			SendCmdBuff.mnum = cmd->mnum;
			SendCmdBuff.ack = 0;
			SendCmdBuff.is_send = 0;
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
	if(NeedToSend)
	{
		if((SendCmd.id && SendCmd.ack) //success
			|| !SendCmd.id  //begin
			||(SendCmd.id && !SendCmd.ack && !SendCmd.is_send))//not send
		{
			//transmit
			exlnk_closeHeader(&Transmit.buffer);

			setTransmitDataSpiDevSec(Transmit.buffer.data, Transmit.buffer.pt_data);

			transmitSpiDevSec();
			NeedToSend = 0;
			Transmit.counter++;
			exlnk_clearSetHeader(&Transmit.buffer);
			//new
			exlnk_initHeader(&Transmit.buffer, Transmit.dma);
			exlnk_fillHeader(&Transmit.buffer, MLINER_SEC_MODULE_ADDRESS, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, Transmit.counter, 0);
			SendCmd.is_send = 1;
		}
		else
		{
			repeatTransmitSpiDevSec();
			NeedToSend = 0;
			if(Receive.repeataction_on)
				Receive.repeataction(SendCmd.id, SendCmd.mnum);
		}
	}
	else
	{
		//receive
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
		receiveSpiDevSec();
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
			if(Receive.buffer.adr == MLINER_SEC_MODULE_ADDRESS)//TODO: move to root
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

				if (SendCmd.id && ack.mnum == SendCmd.mnum)
				{
					SendCmd.ack = 1;
					if(SendCmdBuff.id)
					{
						SendCmd = SendCmdBuff;
						SendCmdBuff.id = 0;
					}
				}
			}
		}
		else
			break;
		len = exlnk_isEmptyGetHeader(&Receive.buffer);
	}
	//check
	if(Receive.counter >= 999)
	{
		if(Receive.onreset_on)
			Receive.onreset();
		ipl_disable();
		Receive.counter = 0;
		ipl_enable();
		disableSpiDevSec();
	}
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
