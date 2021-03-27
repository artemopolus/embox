#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include <tim/tim.h>

#define MAX_CALL_COUNT 10



thread_control_t MainThread;

uint8_t MarkerThread = 0;

uint8_t MarkerTx = 0;
uint8_t MarkerRx = 0;


#define BUFFER_LENGTH 16

uint8_t DataToBuffer[BUFFER_LENGTH] = {0};
uint8_t ReceivedData[BUFFER_LENGTH] = {0};

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;

struct lthread DownLoadDataFromBufferThread;

struct lthread PrintDataFromBufferThread;

struct lthread CheckTransmitThread;
struct lthread CheckReceiveThread;


uint8_t CommonCounter = 0;
uint8_t MarkerSubscribe = 0;
struct lthread SubscribeThread;

uint8_t TickEventMarker = 0;
uint8_t PrintEventMarker = 0;
uint16_t PrintEventCounter = 0;
#define PRINT_EVENT_COUNTER_MAX 10

static int runPrinter(struct  lthread * self)
{
    printf("Print: %d\n", CommonCounter);
    CommonCounter++;
    if (!TickEventMarker)
        TickEventMarker = 1;
    if (!PrintEventMarker)
    {
        if (PrintEventCounter < PRINT_EVENT_COUNTER_MAX)
            PrintEventCounter++;
        else
        {
            PrintEventMarker = 1;
            PrintEventCounter = 0;
        }
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

static int checkTransmitRun(struct lthread * self )
{
    MarkerTx = checkTxSender();
    return 0;
}
static int checkReceiveRun(struct lthread * self)
{
    MarkerRx = checkRxGetter();
    return 0;
}

static int printBufferData(struct  lthread * self)
{
    printf("\33[2K\r");
    printf("\33[2K\r");
    printf("Received buffer: ");
    for (uint8_t i = 0; i < BUFFER_LENGTH; i++)
    {
        printf("[%#04x = %d] ", ReceivedData[i], ReceivedData[i]);
    }
    printf("\nCounter: %d", CommonCounter);
    return 0;
}

static int downloadDataRun(struct lthread * self)
{
    getDataFromExactoDataStorage(ReceivedData, BUFFER_LENGTH);
    return 0;
}
static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, BUFFER_LENGTH); 
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

void ReportData()
{

}


void receiveDataFromStorage()
{
        lthread_launch(&SendDataThread);
        lthread_launch(&CheckReceiveThread);
        if (MarkerRx)
        {
            receiveExactoDataStorage();
            lthread_launch(&DownLoadDataFromBufferThread);
            // lthread_launch(&PrintDataFromBufferThread);
            MarkerRx = 0;
        }

}

int main(int argc, char *argv[]) {

    lthread_init(&SubscribeThread, runSubcribeThread);

    lthread_launch(&SubscribeThread);

    

    MarkerThread = 0;
    printf("Start Full Duplex SPI\n");
    lthread_init(&MarkerCheckerThread, checkMarkerThreadRun);
    printf("Reset ALL\n");
    resetExactoDataStorage();
    printf("Init thread:\n-main\n");
    initThreadExactoDataStorage(&MainThread);
    printf("-printf\n");
    lthread_init(&PrintThread, printThreadRun);
    printf("-sending\n");
    lthread_init(&SendDataThread, sendDataThreadRun);
    printf("Init buffer: \n-upload\n");
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
    printf("-download\n");
    lthread_init(&DownLoadDataFromBufferThread, downloadDataRun);
    printf("-printing\n");
    lthread_init(&PrintDataFromBufferThread, printBufferData);
    lthread_launch(&PrintDataFromBufferThread);
    printf("Upload data to buffer\n");
    printf("Data[ buffer] = > SPI[TX]\n");

    printf("Init threat for RX TX values control\n");
    lthread_init(&CheckReceiveThread, &checkReceiveRun);
    lthread_init(&CheckTransmitThread, &checkTransmitRun);

    printf("Run cycle for checking:\n");
        printf("Try \n" );
        lthread_launch(&UpdateDataToBufferThread);

    while (1)
    {
        while(!TickEventMarker) {}

        receiveDataFromStorage();

        if (PrintEventMarker)
        {
            lthread_launch(&PrintThread);
            PrintEventMarker = 0;
        }

        TickEventMarker = 0;
    }
    
    
    lthread_launch(&PrintThread);


    printf("Programm reach end\n");
    return 0;
}
