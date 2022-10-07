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

  
#include "commander/exacto_data_storage.h"
#include "mliner/main_mod.h"



int main(int argc, char *argv[]) 
{
    int index_max = -1, var_cnt = 3;
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
            printf("Usage:\ntest_read -- infinity number of readings\ntest_read -M [number of reading iterations]\n");
           return 0; 
        }
	} 
    printf("Test read state\n");
    int index = 0;
    while(1)
    {
        uint32_t * val = 0;
        uint8_t res = 0;
        if (var_cnt == 3)
        {
            getMlinerVars(DATACHECK_COUNTER, val, &res );
            printf("[%d]Datacheck: %d \n", index, *val);
            getMlinerVars(TX_BUFFER, val, &res );
            printf("TX: %d \n", *val);
            getMlinerVars(RX_BUFFER, val, &res );
            printf("RX: %d \n", *val);
        }
        index++;
        if (index_max != -1)
        {
            if (index > index_max)
                break;
        }
        usleep(1000000);
    }
    printf("Done\n");
    return 0;
}
