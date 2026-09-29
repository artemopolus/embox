#include <stdio.h>
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

#include "writer_smpl.h"

#include "operator/exacto_buffer.h"
	
ExactoBufferUint8Type ReaderStore;

static uint8_t buffer[1048];
static uint8_t Print2SDFlag;
static int PrintRes;
static uint8_t BBBFlag = 0;

static struct thread *MainBasicThread;

uint16_t ReaderAddCounter;
uint16_t ReaderDataLen;
uint16_t FileDataLen;


static void *runMainBasicThread(void *arg) {
    
	printf("Start thread\n");

    while(BBBFlag == 0)
        sleep(1);
    int	Pt = open("/mnt/test.txt",O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0666);
	if (0 > Pt)
	{
      	printf("Can't open Data file\n");
		return NULL;
	}
	// else
    //   printf("Data file is opened\n");
	Print2SDFlag = 1;
	while (BBBFlag)
	{
		while (Print2SDFlag == 0)
		{
		}

		if (getlen_exbu8(&ReaderStore))
		{
			printf("Write data to file\n");
			Print2SDFlag = 0;
			uint16_t copy = grbfstPack_exbu8(&ReaderStore, buffer, 128);
			PrintRes = write (Pt, buffer, copy);
			Print2SDFlag = 1;
		}
		usleep(1000000);
	}
	

    
	return NULL;
}
void openFileSD()
{
    BBBFlag = 1;
}
uint8_t isReadyToWrite()
{
	return Print2SDFlag;
}
void addDataToWrite( uint8_t * data, uint16_t datalen)
{
	Print2SDFlag = 0;
	ReaderAddCounter ++;
	ReaderDataLen += datalen;
	pshsftPack_exbu8(&ReaderStore, data, datalen);
	Print2SDFlag = 1;
}
void printReaderData()
{
	printf("Thread started: %d\nAdd Data Events Count: %d\nData added len: %d\n", BBBFlag, ReaderAddCounter, ReaderDataLen);
}
EMBOX_UNIT_INIT(initTestSmplMod);
static int initTestSmplMod()
{
	Print2SDFlag = 1;
    BBBFlag = 0;
	ReaderAddCounter = 0;
	ReaderDataLen = 0;
	setini_exbu8(&ReaderStore);
	MainBasicThread = thread_create(THREAD_FLAG_DETACHED |THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);
    thread_launch(MainBasicThread);
	return 0;
}
