#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/lthread/lthread.h>
#include <kernel/thread.h>
#include <kernel/time/ktime.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/thread/sync/mutex.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <embox/unit.h>

#include "smpl_mod.h"

static uint8_t buffer[1048];
uint8_t Print2SDFlag;
int PrintRes;
static uint8_t BBBFlag = 0;

static struct thread *MainBasicThread;


static void *runMainBasicThread(void *arg) {
    
	printf("Start thread\n");

    while(BBBFlag == 0)
        sleep(1);
	Print2SDFlag = 0;
    int	Pt = open("/mnt/test.txt",O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0666);
	if (0 > Pt)
	{
    //   printf("Can't open Data file\n");
		return NULL;
	}
	// else
    //   printf("Data file is opened\n");

    
    uint8_t ck = 0;
    for (int i = 0; i < 1048; i++)
    {
        ck++;
		  buffer[i] = ck;
	 }
   PrintRes = write (Pt, buffer, 1048);

	Print2SDFlag = 1;
	//printf("Done: %d\n", Res);
	return NULL;
}
void startSmplMod()
{

    BBBFlag = 1;
}
EMBOX_UNIT_INIT(initTestSmplMod);
static int initTestSmplMod()
{
	Print2SDFlag = 0;
    BBBFlag = 0;
	MainBasicThread = thread_create(THREAD_FLAG_DETACHED |THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);
    thread_launch(MainBasicThread);
	return 0;
}
