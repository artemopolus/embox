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
#include <kernel/time/ktime.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/lthread/lthread.h>
#include <kernel/task.h>
#include <sys/wait.h>

#include "tim/tim.h"
#include "commander/exacto_filemanager.h"


uint8_t Counter = 0;

uint8_t MarkerSubscribe = 0;

struct lthread SubscribeThread;
struct lthread MainLightThread;

static struct thread *BasicThread;
static cond_t SignalFromTim;
static struct mutex MutexTim;

#define MAX_DIVIDER 10
uint8_t DividerIndex = 0;


static void *runBasicThread(void *arg) {
    printf("Basic full thread run\n");
    uint8_t buffer0[] = "type typ type from common thread\n";

    while(1)
    {
        printf("Try to save to file\n");
        ex_saveToFile(buffer0, sizeof(buffer0));
        mutex_lock(&MutexTim);
        cond_wait(&SignalFromTim, &MutexTim);
    }
	return NULL;
}


static int runMainLightThread(struct lthread * self)
{
	if (mutex_trylock_lthread(self, &MutexTim) == -EAGAIN) {
		return 0;
	}
    printf("Light thread run\n");
    cond_signal(&SignalFromTim);
	// test_emit('d');
	mutex_unlock_lthread(self, &MutexTim);
    // uint8_t buffer0[] = "type typ type from light thread\n";

    // ex_saveToFile(buffer0, sizeof(buffer0));
    return 0;
}
static int runPrinter(struct  lthread * self)
{
    printf("Print: %d\n", Counter);
    Counter++;
    if (DividerIndex < MAX_DIVIDER)
    {
        DividerIndex++;
    }
    else
    {
        DividerIndex = 0;
        lthread_launch(&MainLightThread);
    }
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

	BasicThread = thread_create(THREAD_FLAG_SUSPENDED, runBasicThread, NULL);
	mutex_init(&MutexTim);
	cond_init(&SignalFromTim, NULL);

    thread_launch(BasicThread);
    thread_join(BasicThread, NULL);

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