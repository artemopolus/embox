#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "spi/spi2_generated.h"
#include "gpio/gpio.h"

#define MAX_CALL_COUNT 10



thread_control_t MainThread;

uint8_t MarkerThread = 0;

uint8_t MarkerTx = 0;
uint8_t MarkerRx = 0;
#define DATA_MESSAGE_SIZE 16
uint8_t DataToBuffer[] = {3, 7, 2, 10, 1,
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0};
uint8_t ReceivedData[DATA_MESSAGE_SIZE] = { 0};

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;

struct lthread DownLoadDataFromBufferThread;

struct lthread PrintDataFromBufferThread;

struct lthread CheckTransmitThread;
struct lthread CheckReceiveThread;

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
    printf("Received buffer: ");
    for (uint8_t i = 0; i < DATA_MESSAGE_SIZE; i++)
    {
        printf("%d ", ReceivedData[i]);
    }
    printf("\n");
    return 0;
}

static int downloadDataRun(struct lthread * self)
{
    getDataFromExactoDataStorage(ReceivedData, 16);
    return 0;
}

static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, 16); 
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
    printf("Waiting for spi:");
    while(1)
    {
        usleep(10000);
        if (ex_checkGpio())
        {
            break;
        }
    }
    setupSPI2_FULL_DMA();
    printf("Done\n");

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
    uint8_t pt = 0;
    const uint8_t pt_max = 50;
    uint32_t call_counter = 0;
    while(call_counter < MAX_CALL_COUNT)
    {
        printf("Try %d\n", call_counter);
        lthread_launch(&UpdateDataToBufferThread);
        lthread_launch(&SendDataThread);
        printf("Wait Tx\n");
        uint16_t counter = 0;
        const uint16_t counter_max = 10;
        while (!MarkerTx)
        {
            lthread_launch(&CheckTransmitThread);
            usleep(5000);
            if (pt < pt_max)
            {
                pt++;
                printf(".");
            }
            else{
                pt = 0;
                if (counter > counter_max)
                    break;
                else
                    counter++;
                printf("\33[2K\r");
            }
        }
        printf("\nWait Rx\n");
        counter = 0;

        while (!MarkerRx)
        {
            lthread_launch(&CheckReceiveThread);
            usleep(5000);
            if (pt < pt_max)
            {
                pt++;
                printf(".");
            }
            else{
                pt = 0;
                if (counter > counter_max)
                    break;
                else
                    counter++;
                printf("\33[2K\r");
            }
        }
        
        MarkerTx = 0;
        MarkerRx = 0;
        printf("Copy data from RX\n");
        receiveExactoDataStorage();
        printf("Download data from data storage\n");
        lthread_launch(&DownLoadDataFromBufferThread);
        lthread_launch(&PrintDataFromBufferThread);
        usleep(1000000);
        call_counter++;
    }
    
    lthread_launch(&PrintThread);

    turnOffSPI2_FULL_DMA();


    printf("Programm reach end\n");
    return 0;
}

