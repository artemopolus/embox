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


static uint8_t Command[40] = {1,17,1,17,1,17,0};

int main(int argc, char *argv[]) {
    printf("Test one command\n");
    ex_setExactolinkType(EXACTOLINK_SNS_XLXLGR);

    setDataToExactoDataStorage(Command, 0, EX_THR_CTRL_INIT); 

    setDataToExactoDataStorage(Command, 40, EX_THR_CTRL_WAIT);

    setDataToExactoDataStorage(Command, 0, EX_THR_CTRL_OK); 

    while (!checkTxSender())
    {
        sleep(1);
    }
    
    transmitExactoDataStorage();
    
    while (!checkTxSender())
    {
        printf("One try...\n");
        transmitExactoDataStorage();
        sleep(1);
    }

    printf("Done\n");
    return 0;
}
