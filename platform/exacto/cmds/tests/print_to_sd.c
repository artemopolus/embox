#include <stdio.h>
#include <stdint.h>
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

struct lthread MainLightThread;
static struct thread *MainBasicThread;
static void *runMainBasicThread(void *arg) {
	return NULL;
}
static int runMainLightThread(struct lthread * self)
{
    return 0;
}

int main(int argc, char *argv[]) {

    initExactoFileManager();

    printf("Try to write:\n");

    ex_writeToLogChar("Hello from test print sd\n");

    uint8_t buffer0[] = "this is the first line of text\n";
    uint8_t buffer1[] = "type typ type\n";

    ex_saveToFile(buffer0, sizeof(buffer0));
    ex_saveToFile(buffer1, sizeof(buffer1));

	MainBasicThread = thread_create(THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);


    //printf("I try to read data from sd\n");

    // FILE * p_file;
    // uint8_t buffer[20] = {0};

    // p_file = fopen("/mnt/file.txt", "r");

    // if (p_file != NULL)
    // {
    //     fscanf(p_file, "%s", buffer);
    //     for (uint8_t i = 0; i < 60; i++)
    //         printf("%c",buffer[i]);
    //     fclose(p_file);
    // }
    // else
    // {
    //     printf("Can't open file\n");
    // }

    return 0;
}
