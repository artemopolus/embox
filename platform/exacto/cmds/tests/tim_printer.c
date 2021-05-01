#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/lthread/lthread.h>
#include "commander/exacto_filemanager.h"
#include <util/err.h>
#include <embox/test.h>
#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/lthread/lthread.h>
#include <kernel/thread.h>
#include <kernel/time/ktime.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/thread/sync/mutex.h>
#include "tim/tim.h"

uint8_t Counter = 0;

uint8_t MarkerSubscribe = 0;

struct lthread SubscribeThread;
struct lthread MainLightThread;
static int runMainLightThread(struct lthread * self)
{
    printf("Light thread run\n");
    uint8_t buffer0[] = "type typ type from light thread\n";

    ex_saveToFile(buffer0, sizeof(buffer0));
    return 0;
}
static int runPrinter(struct  lthread * self)
{
    printf("Print: %d\n", Counter);
    Counter++;
    lthread_launch(&MainLightThread);
    return 0;
}
static int runSubcribeThread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runPrinter);
    if (result == 0)
        MarkerSubscribe = 1;
    return 0;
}

int main(int argc, char *argv[]) {
    printf("Start print function\n");
    initExactoFileManager();

    lthread_init(&SubscribeThread, runSubcribeThread);
    lthread_init(&MainLightThread, runMainLightThread);

    lthread_launch(&SubscribeThread);

    printf("Wait subscribing...\n");

    while (!MarkerSubscribe)
    {
    }
    

    while(1)
    {
        usleep(100000);
        printf("ping\n");
    }
    return 0;
}