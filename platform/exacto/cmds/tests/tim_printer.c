#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <util/err.h>

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



#include <sys/wait.h>

#include "tim/tim.h"
#include "commander/exacto_filemanager.h"
#define MAX_DIVIDER 10

uint8_t TmPrDividerIndex = 0;

uint8_t TmPrCounter = 0;

uint8_t TmPrMarkerSubscribe = 0;

static struct lthread TmPrSubscribeThread;
static struct lthread TmPrMainLightThread;

static struct thread *TmPrBasicThread;
static cond_t TmPrSignalFromTim;
static struct mutex TmPrMutexTim;



static void *runTmPrBasicThread(void *arg) {
    printf("Basic full thread run\n");
    uint8_t buffer0[] = "type typ type from common thread\n";

    while(1)
    {
        printf("Try to save to file\n");
        ex_saveToFile(buffer0, sizeof(buffer0));
        mutex_lock(&TmPrMutexTim);
        cond_wait(&TmPrSignalFromTim, &TmPrMutexTim);
        mutex_unlock(&TmPrMutexTim);
    }
	return NULL;
}

static int runTmPrMainLightThread(struct lthread * self)
{
    // start:
    // mutex_retry:
    printk("Light thread run\n");
	if (mutex_trylock_lthread(self, &TmPrMutexTim) == -EAGAIN) {
        // return lthread_yield(&&start, &&mutex_retry);
        return 0;
	}
    printk("after mutex\n");
    cond_signal(&TmPrSignalFromTim);
	// test_emit('d');
	mutex_unlock_lthread(self, &TmPrMutexTim);
    // uint8_t buffer0[] = "type typ type from light thread\n";

    // ex_saveToFile(buffer0, sizeof(buffer0));
    return 0;
}
static int runTmPrPrinter(struct  lthread * self)
{
    TmPrCounter++;
    if (TmPrDividerIndex < MAX_DIVIDER)
    {
        TmPrDividerIndex++;
    }
    else
    {
        printf("Print: %d\n", TmPrCounter);
        TmPrDividerIndex = 0;
        lthread_launch(&TmPrMainLightThread);
    }
    return 0;
}
static int runTmPrSubcribeThread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTmPrPrinter);
    if (result == 0)
        TmPrMarkerSubscribe = 1;
    return 0;
}

int main(int argc, char *argv[]) {
	int l = 200, h = 210;
    printf("Start print function\n");
    initExactoFileManager();

    lthread_init(&TmPrSubscribeThread, runTmPrSubcribeThread);
    lthread_init(&TmPrMainLightThread, runTmPrMainLightThread);

	TmPrBasicThread = thread_create(THREAD_FLAG_SUSPENDED, runTmPrBasicThread, NULL);
	mutex_init(&TmPrMutexTim);
	cond_init(&TmPrSignalFromTim, NULL);
    
    schedee_priority_set(&TmPrMainLightThread.schedee,h);
    schedee_priority_set(&TmPrBasicThread->schedee,l);



    lthread_launch(&TmPrSubscribeThread);

    printf("Wait subscribing...\n");

    while (!TmPrMarkerSubscribe)
    {
    }
    
    thread_launch(TmPrBasicThread);
    thread_join(TmPrBasicThread, NULL);

    while(1)
    {
        usleep(100000);
        printf("ping\n");
    }
    return 0;
}