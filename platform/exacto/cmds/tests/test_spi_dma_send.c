#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "exacto_commander/exacto_data_storage.h"

thread_control_t MainThread;

uint8_t MarkerThread = 0;

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

static int printThreadRun(struct lthread * self)
{
    printf("Test spi done\n");
    return 0;
}
static int checkMarkerThreadRun(struct lthread * self)
{
    if (MainThread.result == THR_CTRL_OK)
    {
        MarkerThread = 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("Start Slave Full Duplex SPI\n");
    lthread_init(&MarkerCheckerThread, checkMarkerThreadRun);
    printf("Init main thread\n");
    initThreadExactoDataStorage(&MainThread);
    printf("Init printf thread\n");
    lthread_init(&PrintThread, printThreadRun);
    printf("Run cycle for checking\n");
    while (!MarkerThread)
    {
        checkExactoDataStorage(&MainThread);
        lthread_launch(&MarkerCheckerThread);
        sleep(1);
    }
    lthread_launch(&PrintThread);

    printf("Programm reach end\n");
    return 0;
}

