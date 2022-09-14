#include "mliner_sec.h"

#include "commander/exacto_buffer.h"
#include "commander/exacto_services.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"

#include "tim/tim.h"

#include "spi/spi_mline_sec.h"

#include <string.h>

#define MLINER_SEC_MSG_SIZE EXACTO_BUFFER_UINT8_SZ
#define MLINER_SEC_MODULE_ADDRESS	7 


static uint8_t NeedToSend = 0;

// static exlnk_set_header_str_t SendBuffer;
// static uint32_t ECTM_SendData_Counter = 0;
// static uint8_t ECTM_TransmitBuffer[MLINER_SEC_MSG_SIZE] = {0};

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
	uint8_t onreset_on;
	int(*onreset)();
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
		exlnk_CmdToArray((exlnk_cmd_str_t*)data, TmpBuffer, 100);
	else
		return;
	exlnk_uploadHeader(&Transmit.buffer, TmpBuffer, len);
}
void exmliner_Update()
{
	if(NeedToSend)
	{
		//transmit
		exlnk_closeHeader(&Transmit.buffer);

		setTransmitDataSpiDevSec(Transmit.buffer.data, Transmit.buffer.pt_data);

		if(transmitSpiDevSec())
			NeedToSend = 0;
		Transmit.counter++;
		exlnk_clearSetHeader(&Transmit.buffer);
		//new
		exlnk_initHeader(&Transmit.buffer, Transmit.dma);
		exlnk_fillHeader(&Transmit.buffer, MLINER_SEC_MODULE_ADDRESS, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, Transmit.counter, 0);

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
			
		if(exlnk_getCmd(&in, &Receive.buffer.data[Receive.buffer.datapt], Receive.buffer.datalen))
		{
			Receive.buffer.datapt += sizeof( exlnk_cmd_str_t);
			//process
			if(Receive.cmdaction_on)
				Receive.cmdaction(&in);
			// if(GettBuffer.adr == SEC_MODULE_ADDRESS)
			// {
			// 	in.value +=3;
			// 	uploadToMline(&in, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD);
			// 	NeedToSend = 1;
			// }
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
void exmliner_setResetAction(int(*resetaction)())
{
	Receive.onreset = resetaction;
	Receive.onreset_on = 1;
}

