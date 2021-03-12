#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"

thread_control_t MainThread;

uint8_t MarkerThread = 0;
uint8_t DataToBuffer[] = {0, 7, 2, 10, 1};
uint8_t ReceivedData[] = {0, 0, 0, 0, 0};

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;

struct lthread DownLoadDataFromBufferThread;

struct lthread PrintDataFromBufferThread;

static int printBufferData(struct  lthread * self)
{
    printf("Received buffer: ");
    for (uint8_t i = 0; i < 5; i++)
    {
        printf("%d ", ReceivedData[i]);
    }
    printf("\n");
    return 0;
}

static int downloadDataRun(struct lthread * self)
{
    getDataFromExactoDataStorage(ReceivedData, 5);
    return 0;
}

static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, 5); 
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
    MarkerThread = 0;
    printf("Start Slave Full Duplex SPI\n");
    lthread_init(&MarkerCheckerThread, checkMarkerThreadRun);
    printf("Reset ALL\n");
    resetExactoDataStorage();
    printf("Init main thread\n");
    initThreadExactoDataStorage(&MainThread);
    printf("Init printf thread\n");
    lthread_init(&PrintThread, printThreadRun);
    printf("Init sending thread\n");
    lthread_init(&SendDataThread, sendDataThreadRun);
    printf("Init buffer: \n upload\n");
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
    printf("download\n");
    lthread_init(&DownLoadDataFromBufferThread, downloadDataRun);
    printf("printing\n");
    lthread_init(&PrintDataFromBufferThread, printBufferData);
    lthread_launch(&PrintDataFromBufferThread);
    printf("Run cycle for checking:\n");
    uint8_t pt = 0;
    const uint8_t pt_max = 50;
    while (!MarkerThread)
    {
        checkExactoDataStorage(&MainThread);
        lthread_launch(&MarkerCheckerThread);
        usleep(100000);
        
        if (pt < pt_max)
        {
            pt++;
            printf(".");
        }
        else{
            pt = 0;
            printf("\33[2K\r");
        }

    }
    lthread_launch(&PrintThread);

    lthread_launch(&PrintDataFromBufferThread);
    

    printf("Update buffer to send\n");

    lthread_launch(&UpdateDataToBufferThread);

    printf("Send some answer\n");

    lthread_launch(&SendDataThread);

    printf("Programm reach end\n");
    return 0;
}

