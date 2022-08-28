#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/thread.h>
#include <kernel/thread/sync/mutex.h>
#include <kernel/thread/sync/cond.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/task.h>
#include <kernel/time/ktime.h>
#include <kernel/printk.h>


#include "tim/tim.h"
#include "commander/exacto_data_storage.h"
#include "exlnk_setHeader.h"
#include "exlnk_getHeader.h"

#define ECTM_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
uint8_t ECTM_TransmitBuffer[ECTM_MESSAGE_SIZE] = {0};
uint8_t ECTM_ReceiveBuffer[ECTM_MESSAGE_SIZE] = {0};
static uint32_t ECTM_SendData_Counter = 0;

exlnk_set_header_str_t SendBuffer;
exlnk_get_header_str_t GettBuffer;

static void sending(int value)
{
    exds_getData(ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, 0); 

    exlnk_getHeader(&ECTM_ReceiveBuffer, ECTM_MESSAGE_SIZE, &GettBuffer);

    ECTM_SendData_Counter++;
    exds_setData(SendBuffer.data, SendBuffer.pt_data, EX_THR_CTRL_OK);

    transmitExactoDataStorage();
}
static void init()
{
    exlnk_initHeader(&SendBuffer, ECTM_TransmitBuffer);
    exlnk_fillHeader(&SendBuffer, 1, EXLNK_MSG_SIMPLE, EXLNK_PACK_SIMPLE, 0, ECTM_SendData_Counter, 0);
    exlnk_closeHeader(&SendBuffer);
}
int main(int argc, char *argv[]) 
{
    int index_max = -1, var_cnt = 2;
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
    while(index_max == 0)
    {
        for(int i = 0; i < var_cnt; i++)
            sending(i);
        index_max --;
    }
    return 1;
}