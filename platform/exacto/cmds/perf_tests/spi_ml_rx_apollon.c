#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "spi/spi_mliner.h"
#include "gpio/gpio.h"
#include "ex_utils.h"

#define MAX_CALL_COUNT 10



thread_control_t MainThread;

uint8_t MarkerThread = 0;

uint8_t MarkerTx = 0;
uint8_t MarkerRx = 0;
#define DATA_MESSAGE_SIZE 16
uint8_t DataToBuffer[] = {17, 7, 2, 10, 1,
                            0, 0, 0, 0, 0,
                            0, 3, 3, 3, 3, 3};
uint8_t ReceivedData[DATA_MESSAGE_SIZE] = { 0};

static struct lthread MarkerCheckerThread;

static struct lthread UpdateDataToBufferThread;
static struct lthread SendDataThread;

static struct lthread DownLoadDataFromBufferThread;


static struct lthread CheckTransmitThread;
static struct lthread CheckReceiveThread;

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

int printBufferData()
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
    // getDataFromExactoDataStorage(ReceivedData, SPI_MLINER_BUFFER_SIZE);
    uint16_t pack_len;
    ex_getPack_ExactoDtStr(ReceivedData, sizeof(ReceivedData),&pack_len,EX_XL_LSM303AH);
    return 0;
}

static int updateDataToBufferThreadRun(struct lthread * self)
{
    // setDataToExactoDataStorage(DataToBuffer, SPI_MLINER_BUFFER_SIZE, THR_CTRL_OK); 
    ex_setData_ExactoDtStr(DataToBuffer, sizeof(DataToBuffer), 1, EX_XL_LSM303AH);
    return 0;
}
static int sendDataThreadRun(struct lthread * self)
{
    transmitExactoDataStorage();
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
    resetExactoDataStorage();
    initThreadExactoDataStorage(&MainThread);
    lthread_init(&SendDataThread, sendDataThreadRun);
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
    lthread_init(&DownLoadDataFromBufferThread, downloadDataRun);

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
        printBufferData();
        usleep(1000000);
        call_counter++;
    }
    

    turnOffSPI2_FULL_DMA();


    printf("Programm reach end\n");
    return 0;
}

