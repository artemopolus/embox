#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"

uint8_t Counter = 0;

uint8_t MarkerSubscribe = 0;

struct lthread SubscribeThread;

static int runPrinter(struct  lthread * self)
{
    printf("Print: %d\n", Counter);
    Counter++;
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

    lthread_init(&SubscribeThread, runSubcribeThread);

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