#include <stdio.h>
#include <stdint.h>
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

static uint8_t buffer[1048];
static uint8_t Flag;
static uint8_t Res;
int Pt;
static struct thread *MainBasicThread;


static void *runMainBasicThread(void *arg) {
	printf("Start thread\n");
    uint8_t ck = 0;
    for (int i = 0; i < 1048; i++)
    {
        ck++;
		  buffer[i] = ck;
	 }
   Res = write (Pt, buffer, 1048);

	Flag = 1;
	return NULL;
}

int main(int argc, char *argv[]) 
{
	Flag = 0;
	Pt = open("/mnt/test.txt",O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0666);
	if (0 > Pt)
	{
      printf("Can't open Data file\n");
		return 1;
	}
	else
      printf("Data file is opened\n");

	MainBasicThread = thread_create(THREAD_FLAG_DETACHED |THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);
   thread_launch(MainBasicThread);

	while(Flag == 0)
		sleep(1);

	printf("Done: %d\n", Res);

	return 0;
}
