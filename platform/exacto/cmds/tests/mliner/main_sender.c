#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/printk.h>


#include "tim/tim.h"
#include "commander/exacto_data_storage.h"
#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"
#include "exlnk_Cmd.h"

#define ECTM_MESSAGE_SIZE 2*EXACTO_BUFFER_UINT8_SZ
static uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;

#define ECTM_SEC_COUNT 2

static uint8_t Addresses[] = {7,16};
static exlnk_set_header_str_t SendBuffer[ECTM_SEC_COUNT];
static exlnk_get_header_str_t GettBuffer;

static uint8_t TmpBuffer[100];

// uint8_t AddressSendOrder[] = {7, 7, 7, 7, 7, 0,0,0,0,0, 16, 16, 16, 16, 16, 0,0,0,0,0};
// uint8_t AddressSendOrder[] = {7, 0,0, 16, 0,0};
static uint8_t AddressSendOrder[] = {7, 16, 7, 16, 7, 16};
static uint8_t AddressCount = 6;

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
    exlnk_closeHeader(trg);

    exds_setData(trg->data, trg->pt_data, EX_THR_CTRL_OK);
    
    ECTM_SendData_Counter++;

}
static void sending(uint8_t value)
{
    for (int i = 0; i < ECTM_SEC_COUNT; i++)
    {
        if(SendBuffer[i].is_closed)
        {
            exlnk_initHeader(&SendBuffer[i], &ECTM_TransmitBuffer[i*EXACTO_BUFFER_UINT8_SZ]);
            exlnk_fillHeader(&SendBuffer[i], Addresses[i], EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);
        }
    }
    

    exds_getData(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, 0); 

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
			GettBuffer.datapt += sizeof( exlnk_cmdack_str_t);
        }
        else
            break;
        len = exlnk_isEmptyGetHeader(&GettBuffer);
    }
    printf("\n");


    prepareTransmit(value);

    transmitExactoDataStorage();
}
static void init()
{
    ECTM_SendData_Counter = 0;
    ex_setExactolinkType(EXACTOLINK_CMD_COMMON);
    for (int i = 0; i < ECTM_SEC_COUNT; i++)
    {
        exlnk_initHeader(&SendBuffer[i], &ECTM_TransmitBuffer[i*EXACTO_BUFFER_UINT8_SZ]);
        exlnk_fillHeader(&SendBuffer[i], Addresses[i], EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);
    }
}
int main(int argc, char *argv[]) 
{
    int index_max = -1;
    int var_cnt = 2;
	int opt;
    extern char * optarg;
	while((opt = getopt(argc, argv, "M:f:h")) != -1)
    {
		switch (opt) {
		case 'M':
            if (optarg) 
                index_max = atoi(optarg);
			break;
		case 'f':
            if (optarg) 
                var_cnt = atoi(optarg);
            break;
		case 'h':
            printf(
                "Usage:\ntest_read -- infinity number of readings\ntest_read -M [number of reading iterations, default = inf]\n-f [number of device to search, default = 2]");
           return 0; 
        }
	}
    init();
    uint8_t i = 0;
    printf("%d", var_cnt);
    while(index_max != 0)
    {
        sending(AddressSendOrder[i++]);
        if(i >= AddressCount)
        {
            i = 0;
        }
        index_max --;
        sleep(2);
    }
    return 1;
}