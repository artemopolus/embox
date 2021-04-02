#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "spi/spi2_generated.h"
#include "gpio/gpio.h"
#include "tim/tim.h"

#define SPI_TXRX_PRINT_ON

#define MAX_CALL_COUNT 10
#define EVENT_SEND_TICKER_MAX 9
uint16_t MlineEventSendTicker = 0;
uint16_t MlineSensorTickerCounter = 0;
uint8_t MlineMarkerSubscribe = 0;
uint8_t MlineMarkerEnd = 0;
uint8_t MlineMarkerSend = 0;
//0x01 -- SubscribeOn
//0x02 -- EndMarkerOn
//0x04 -- SendMarkerOn
struct lthread MlineSubscribeThread;


thread_control_t MainThread;

uint8_t MarkerThread = 0;

uint8_t MarkerTx = 0;
uint8_t MarkerRx = 0;
#define DATA_MESSAGE_SIZE 64
uint8_t DataToBuffer[DATA_MESSAGE_SIZE] = { 0};
uint8_t ReceivedData[DATA_MESSAGE_SIZE] = { 0};

struct lthread PrintThread;
struct lthread MarkerCheckerThread;

struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;

struct lthread DownLoadDataFromBufferThread;

struct lthread PrintDataFromBufferThread;

struct lthread CheckTransmitThread;
struct lthread CheckReceiveThread;

static int runMlineSensorTickerThread(struct lthread * self)
{
    MlineSensorTickerCounter++;
    if (!MlineMarkerSend){
        if (MlineEventSendTicker < EVENT_SEND_TICKER_MAX) 
        {
            MlineEventSendTicker++;
                    }
        else
        {
            MlineMarkerSend = 1;
            MlineEventSendTicker = 0;

        }
    }
    return 0;
}
static int runMlineSubscribeThread(struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runMlineSensorTickerThread);
    if (result == 0)
        MlineMarkerSubscribe = 1;
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
#ifdef SPI_TXRX_PRINT_ON
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    for (uint8_t i = 0; i < DATA_MESSAGE_SIZE; i++)
    {
        printf("%#04x|", ReceivedData[i]);
    }
    printf("\nCounter: %d\n", MlineSensorTickerCounter);
#endif
    return 0;
}

static int downloadDataRun(struct lthread * self)
{
    getMailFromExactoDataStorage(ReceivedData, DATA_MESSAGE_SIZE);
    // getDataFromExactoDataStorage(ReceivedData, DATA_MESSAGE_SIZE);
    return 0;
}

static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, DATA_MESSAGE_SIZE); 
    return 0;
}
static int sendDataThreadRun(struct lthread * self)
{
    transmitExactoDataStorage();
    return 0;
}
static int printThreadRun(struct lthread * self)
{
#ifdef SPI_TXRX_PRINT_ON
    printf("Test spi done\n");
#endif
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

#ifdef SPI_TXRX_PRINT_ON
    printf("Start Full Duplex SPI\n");
#endif
    lthread_init(&MarkerCheckerThread, checkMarkerThreadRun);
#ifdef SPI_TXRX_PRINT_ON
    printf("Reset ALL\n");
#endif
    resetExactoDataStorage();
#ifdef SPI_TXRX_PRINT_ON
    printf("Init thread:\n-main\n");
#endif
    initThreadExactoDataStorage(&MainThread);
#ifdef SPI_TXRX_PRINT_ON
    printf("-printf\n");
#endif
    lthread_init(&PrintThread, printThreadRun);
#ifdef SPI_TXRX_PRINT_ON
    printf("-sending\n");
#endif
    lthread_init(&SendDataThread, sendDataThreadRun);
#ifdef SPI_TXRX_PRINT_ON
    printf("Init buffer: \n-upload\n");
#endif
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
#ifdef SPI_TXRX_PRINT_ON
    printf("-download\n");
#endif
    lthread_init(&DownLoadDataFromBufferThread, downloadDataRun);
#ifdef SPI_TXRX_PRINT_ON
    printf("-printing\n");
#endif
    lthread_init(&PrintDataFromBufferThread, printBufferData);
    lthread_launch(&PrintDataFromBufferThread);
#ifdef SPI_TXRX_PRINT_ON
    printf("Upload data to buffer\n");
    printf("Data[ buffer] = > SPI[TX]\n");

    printf("Init threat for RX TX values control\n");
#endif
    lthread_init(&CheckReceiveThread, &checkReceiveRun);
    lthread_init(&CheckTransmitThread, &checkTransmitRun);

#ifdef SPI_TXRX_PRINT_ON
    printf("Run cycle for checking:\n");
#endif
    uint8_t pt = 0;
    const uint8_t pt_max = 50;
    uint32_t call_counter = 0;

//=================================================================================================
#ifdef SPI_TXRX_PRINT_ON
    printf("Subscribe on TIM \n");
#endif
    lthread_init(&MlineSubscribeThread, runMlineSubscribeThread);
    lthread_launch(&MlineSubscribeThread);

#ifdef SPI_TXRX_PRINT_ON
    printf("Waiting for approving of subscribing\n");
#endif

    while(!MlineMarkerSubscribe)
    {
    }

#ifdef SPI_TXRX_PRINT_ON
    printf("Starting observing of sensor data:");
    printf("\nReceived buffer:\n\n\n ");
#endif
    while(!MlineMarkerEnd)
    {
        while(!MlineMarkerSend) {}

        lthread_launch(&DownLoadDataFromBufferThread);

        lthread_launch(&PrintDataFromBufferThread);
        MlineMarkerSend = 0;
    }

    MarkerThread = 0;
#ifdef SPI_TXRX_PRINT_ON
    printf("Waiting for spi:");
#endif
    while(1)
    {
        usleep(10000);
        if (ex_checkGpio())
        {
            break;
        }
    }
    setupSPI2_FULL_DMA();
#ifdef SPI_TXRX_PRINT_ON
    printf("Done\n");
#endif


//=================================================================================================

    while(call_counter < MAX_CALL_COUNT)
    {
#ifdef SPI_TXRX_PRINT_ON
        printf("Try %d\n", call_counter);
#endif
        lthread_launch(&UpdateDataToBufferThread);
        lthread_launch(&SendDataThread);
#ifdef SPI_TXRX_PRINT_ON
        printf("Wait Tx\n");
#endif
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
#ifdef SPI_TXRX_PRINT_ON
                printf("\33[2K\r");
#endif

            }
        }
#ifdef SPI_TXRX_PRINT_ON
        printf("\nWait Rx\n");
#endif
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
#ifdef SPI_TXRX_PRINT_ON
                printf("\33[2K\r");
#endif
            }
        }
        
        MarkerTx = 0;
        MarkerRx = 0;
#ifdef SPI_TXRX_PRINT_ON
        printf("Copy data from RX\n");
#endif
        receiveExactoDataStorage();
#ifdef SPI_TXRX_PRINT_ON
        printf("Download data from data storage\n");
#endif
        lthread_launch(&DownLoadDataFromBufferThread);
        lthread_launch(&PrintDataFromBufferThread);
        usleep(1000000);
        call_counter++;
    }
    
    lthread_launch(&PrintThread);

    turnOffSPI2_FULL_DMA();

#ifdef SPI_TXRX_PRINT_ON

    printf("Programm reach end\n");
#endif
    return 0;
}

