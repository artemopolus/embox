#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_sns_ctrl.h"
#include "spi/spi_mliner.h"
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

uint8_t MlineSpiEnableMarker = 0;


thread_control_t MainThread;

uint8_t MarkerThread = 0;

uint8_t MarkerTx = 0;
uint8_t MarkerRx = 0;
#define DATA_MESSAGE_SIZE 64
uint8_t DataToBuffer[DATA_MESSAGE_SIZE] = { 0};
uint8_t ReceivedData[DATA_MESSAGE_SIZE] = { 0};

struct lthread MarkerCheckerThread;


struct lthread DownLoadDataFromBufferThread;

struct lthread PrintDataFromBufferThread;

void executeSpiTxRxStage();

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
            // MlineMarkerSend = 1;
            executeSpiTxRxStage();
            MlineEventSendTicker = 0;

        }
    }
    return 0;
}
static int runMlineSubscribeThread(struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runMlineSensorTickerThread);
    if (result == 0)
        MlineMarkerSubscribe = 1;
    return 0;
}


static int printBufferData(struct  lthread * self)
{
#ifdef SPI_TXRX_PRINT_ON
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    uint8_t length = DATA_MESSAGE_SIZE;
    uint8_t start_point = 4;
    for (uint8_t i = 0; i < DATA_MESSAGE_SIZE; i++)
    {
        printf("%#04x|", ReceivedData[i]);
        if (i < (DATA_MESSAGE_SIZE - 4))
        {
            if ((ReceivedData[i]== 0x05)&&(ReceivedData[i+1]== 0x05)&&(ReceivedData[i+2]== 0x05)&&(ReceivedData[i+3]== 0x05))
            {
                length = i;
            }
        }
    }
    printf("\n");
    start_point = ReceivedData[EXACTOLINK_START_DATA_POINT_ADR];

    uint64_t ExDtBfCounter = 0;

    convertUint8ToUint64(&ReceivedData[start_point],&ExDtBfCounter);
    printf("\nData output: %d\n", length);
    for (uint8_t i = start_point  + 4; i < length; i+=2)
    {
        int16_t value;
        convertUint8ToInt16(&ReceivedData[i], &value);
        printf("%d\t", value);
    }
    printf("\nCounter: %d ExDtBfCounter: %d SpiOn: %d\n", MlineSensorTickerCounter, ExDtBfCounter, MlineSpiEnableMarker);
#endif
    return 0;
}

static int downloadDataRun(struct lthread * self)
{
    getMailFromExactoDataStorage(ReceivedData, DATA_MESSAGE_SIZE);
    // getDataFromExactoDataStorage(ReceivedData, DATA_MESSAGE_SIZE);
    return 0;
}




static int checkMarkerThreadRun(struct lthread * self)
{
    if (MainThread.result == EX_THR_CTRL_OK)
    {
        MarkerThread = 1;
    }
    return 0;
}

void executeSpiTxRxStage()
{
    lthread_launch(&DownLoadDataFromBufferThread);
    lthread_launch(&PrintDataFromBufferThread);

    if (ex_checkGpio())
    {
        if (!MlineSpiEnableMarker)
        {
            MlineSpiEnableMarker = 1;
            setupSPI2_FULL_DMA();
        }
            transmitExactoDataStorage();
        if (checkTxSender())
        {

        }
        if (checkRxGetter())
        {
            // lthread_launch(&DownLoadDataFromBufferThread);
            // lthread_launch(&PrintDataFromBufferThread);
            receiveExactoDataStorage();
        }
    }
    else
    {
        if (MlineSpiEnableMarker)
        {
            MlineSpiEnableMarker = 0;
            turnOffSPI2_FULL_DMA();
        }
    } 
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
#ifdef SPI_TXRX_PRINT_ON
    printf("-sending\n");
#endif
#ifdef SPI_TXRX_PRINT_ON
    printf("Init buffer: \n-upload\n");
#endif
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

#ifdef SPI_TXRX_PRINT_ON
    printf("Run cycle for checking:\n");
#endif

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
    while(1)
    {
        // while(!MlineMarkerSend) {}

        // lthread_launch(&DownLoadDataFromBufferThread);

        // lthread_launch(&PrintDataFromBufferThread);

        // if (ex_checkGpio())
        // {
        //     lthread_launch(&SendDataThread);
        //     if (checkTxSender())
        //     {

        //     }
        //     if (checkRxGetter())
        //     {
        //         lthread_launch(&DownLoadDataFromBufferThread);
        //         lthread_launch(&PrintDataFromBufferThread);

        //     }
        //     receiveExactoDataStorage();
        // }

        // MlineMarkerSend = 0;
    }

    return 0;
}

