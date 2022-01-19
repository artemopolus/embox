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



int main(int argc, char *argv[]) {
    printf("Test read state\n");
    int index = 0;
    while(1)
    {
        uint32_t * val = 0;
        uint8_t res = 0;
        getMlinerVars(DATACHECK_COUNTER, val, &res );
        printf("[%d]Datacheck: %d \n", index, *val);
        getMlinerVars(TX_BUFFER, val, &res );
        printf("TX: %d \n", *val);
        getMlinerVars(RX_BUFFER, val, &res );
        printf("RX: %d \n", *val);
        index++;
        usleep(1000000);
    }
    printf("Done\n");
    return 0;
}
