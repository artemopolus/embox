#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"

thread_control_t MainThread;

uint8_t MarkerThread = 0;
uint8_t DataToBuffer[] = {0, 7, 2, 10};

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;
static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, 4); 
    return 0;
}
static int sendDataThreadRun(struct lthread * self)
{
    transmitExactoDataStorage();
    return 0;
}
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
    printf("Init sending thread\n");
    lthread_init(&SendDataThread, sendDataThreadRun);
    printf("Init buffer upload\n");
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
    printf("Run cycle for checking\n");
    while (!MarkerThread)
    {
        checkExactoDataStorage(&MainThread);
        lthread_launch(&MarkerCheckerThread);
        sleep(1);
    }
    lthread_launch(&PrintThread);

    printf("Update buffer to send\n");

    lthread_launch(&UpdateDataToBufferThread);

    printf("Send some answer\n");

    lthread_launch(&SendDataThread);

    printf("Programm reach end\n");
    return 0;
}

