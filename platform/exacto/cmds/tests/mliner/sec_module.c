#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <kernel/lthread/lthread.h>
#include <kernel/printk.h>


// #include "commander/exacto_data_storage.h"
#include "commander/exacto_buffer.h"
#include "spi/spi_mline_sec.h"

#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"
#include "exlnk_Cmd.h"

#include "tim/tim.h"

#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
#define SEC_MODULE_ADDRESS	7 

static uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;
static uint16_t ECTM_Trial_Counter = 0;

static uint16_t TIM_Counter = 0;

static exlnk_set_header_str_t SendBuffer;
static exlnk_get_header_str_t GettBuffer;

static uint8_t TmpBuffer[100];

static uint8_t NeedToSend = 0;
static uint8_t NeedToPrint = 0;

static uint16_t LastAddress = 0;

ExactoBufferUint8Type TransmitStore;
ExactoBufferUint8Type ReceiveStore;

static int PointToTim;

static void checking();

static int run_Tim_Lthread(struct  lthread * self)
{
	if(!NeedToSend)
		checking();
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
	return 0;
}
void receiveMline()
{
	if(grball_exbu8(&ReceiveStore, ECTM_ReceiveBuffer))
	{
		memset(&GettBuffer, 0, sizeof(GettBuffer));
		if(exlnk_getHeader(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer))
		{
			//
		}
	}
}
static void checking()
{
	if(!NeedToSend)
	{

		if(grball_exbu8(&ReceiveStore, ECTM_ReceiveBuffer))
		{
			memset(&GettBuffer, 0, sizeof(GettBuffer));
			if(exlnk_getHeader(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer))
			{
				LastAddress = GettBuffer.adr;
				if(GettBuffer.adr == SEC_MODULE_ADDRESS)
				{
					NeedToSend = 1;
				}
				ECTM_Trial_Counter = 0;
			}
			else
			{
				ECTM_Trial_Counter++;
			}
		}
		receiveSpiDevSec();
	}
}
void uploadToMline(void * data, size_t len, uint8_t id)
{
	if(exlnk_isEmptyHeader(&SendBuffer))
	{
		exlnk_initHeader(&SendBuffer, ECTM_TransmitBuffer);
		exlnk_fillHeader(&SendBuffer, SEC_MODULE_ADDRESS, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);
	}
	exlnk_CmdToArray((exlnk_cmd_str_t*)data, TmpBuffer, 100);
	exlnk_uploadHeader(&SendBuffer, TmpBuffer, len);
}
void transmitMline()
{
	exlnk_closeHeader(&SendBuffer);
	for(int i = 0; i < SendBuffer.pt_data; i++)
		pshsft_exbu8(&TransmitStore, SendBuffer.data[i]);

	transmitSpiDevSec();
	ECTM_SendData_Counter++;
	exlnk_clearHeader(&SendBuffer);
}
static void sending()
{
	exlnk_cmd_str_t in;
	exlnk_getCmd(&in, &GettBuffer.data[GettBuffer.datapt], GettBuffer.datalen);

	printf("in<=[adr: %d\tval: %d]\n", in.address, in.value);

	in.value +=3;

	uploadToMline(&in, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD);

	transmitMline();

	NeedToSend = 0;
}
static void init()
{
	printf("My address: %d\n", SEC_MODULE_ADDRESS);
	setini_exbu8(&ReceiveStore);
	setini_exbu8(&TransmitStore);
	setRxBuffSpiDevSec(&ReceiveStore);
	setTxBuffSpiDevSec(&TransmitStore);
	ECTM_SendData_Counter = 0;
	TIM_Counter = 0;
	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);

	ex_setFreqHz(100);
}
int main(int argc, char *argv[]) 
{

    init();
    while(1)
    {
		if(NeedToSend)
		{
			sending();
		}
		if(NeedToPrint)
		{
			printf("send[%d]try[%d]: %d\n", ECTM_SendData_Counter, ECTM_Trial_Counter, LastAddress);
			NeedToPrint = 0;
		}
		if(ECTM_Trial_Counter >= 999)
		{
			printf("Try to reset Mline\n");
			ipl_disable();
			ECTM_Trial_Counter = 0;
			ipl_enable();
			disableSpiDevSec();
		}
    }
    return 1;
}

