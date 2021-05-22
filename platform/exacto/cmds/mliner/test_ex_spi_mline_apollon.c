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
#include "sns_service.h"

#define SPI_TXRX_PRINT_ON

#define MAX_CALL_COUNT 10

uint8_t TESMA_Sender_Counter = 0;
uint8_t TESMA_Sender_Max = 9;

uint32_t TESMA_Tx_Counter = 0;
uint32_t TESMA_Tx_Buffer;
uint32_t TESMA_Rx_Counter = 0;
uint32_t TESMA_Rx_Buffer;



uint8_t TESMA_MlineSpiEnableMarker = 0;

thread_control_t TESMA_MainThread;


#define TESMA_DATA_MESSAGE_SIZE SPI_MLINER_BUFFER_SIZE
uint8_t TESMA_ReceivedData[TESMA_DATA_MESSAGE_SIZE] = { 0};
uint16_t TESMA_ReceivedData_Length = 0;

static struct lthread TESMA_DownloadData_Lthread;
uint8_t               TESMA_DownloadData_Marker = 0;
uint8_t               TESMA_DownloadData_Counter = 0;
uint8_t               TESMA_DownloadData_Max = 9;

void executeSpiTxRxStage();

static int runTESMA_TimReceiver_Lthread(struct lthread * self)
{

    if (TESMA_Sender_Counter < TESMA_Sender_Max)
    {
        TESMA_Sender_Counter++;
    }
    else
    {
        TESMA_Sender_Counter = 0;
        executeSpiTxRxStage();
    }

    return 0;
}

void printBufferData()
{
#ifdef SPI_TXRX_PRINT_ON
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    // uint8_t length = TESMA_DATA_MESSAGE_SIZE;
    // uint8_t start_point = 4;
    // for (uint8_t i = 0; i < TESMA_DATA_MESSAGE_SIZE; i++)
    // {
    //     printf("%#04x|",TESMA_ReceivedData[i]);
    //     if (i < (TESMA_DATA_MESSAGE_SIZE - 4))
    //     {
    //         if ((TESMA_ReceivedData[i]== 0x05)&&(TESMA_ReceivedData[i+1]== 0x05)&&(TESMA_ReceivedData[i+2]== 0x05)&&(TESMA_ReceivedData[i+3]== 0x05))
    //         {
    //             length = i;
    //         }
    //     }
    // }
    // printf("\n");
    // start_point = TESMA_ReceivedData[EXACTOLINK_START_DATA_POINT_ADR];

    // uint64_t ExDtBfCounter = 0;

    // convertUint8ToUint64(&TESMA_ReceivedData[start_point],&ExDtBfCounter);
    printf("\nData output: %d\n", TESMA_DATA_MESSAGE_SIZE);
    for (uint8_t i = 0; i < 6; i+=2)
    {
        int16_t value;
        convertUint8ToInt16(&TESMA_ReceivedData[i], &value);
        printf("%d\t", value);
    }
    printf("\n");
    printf("SpiOn: %d| Tx: %d| Rx: %d|\n", TESMA_MlineSpiEnableMarker, TESMA_Tx_Buffer, TESMA_Rx_Buffer);
    printf("Len: %d\n",TESMA_ReceivedData_Length);
#endif
    return; 
}

static int runTESMA_DownloadData_Lthread(struct lthread * self)
{
    if(!TESMA_DownloadData_Marker)
    {
        // getMailFromExactoDataStorage(TESMA_ReceivedData, TESMA_DATA_MESSAGE_SIZE);
        ex_getPack_ExactoDtStr(TESMA_ReceivedData, TESMA_DATA_MESSAGE_SIZE, &TESMA_ReceivedData_Length, EX_XL_LSM303AH);
        TESMA_DownloadData_Marker = 1;
        TESMA_Rx_Buffer = TESMA_Rx_Counter;
        TESMA_Tx_Buffer = TESMA_Tx_Counter;
    }
    return 0;
}

void executeSpiTxRxStage()
{
    if (TESMA_DownloadData_Counter < TESMA_DownloadData_Max)
    {
        TESMA_DownloadData_Counter++;
    }
    else
    {
        lthread_launch(&TESMA_DownloadData_Lthread );
        TESMA_DownloadData_Counter = 0;
    }

    if (ex_checkGpio())
    {
        if (!TESMA_MlineSpiEnableMarker)
        {
            TESMA_MlineSpiEnableMarker = 1;
            setupSPI2_FULL_DMA();
        }
            transmitExactoDataStorage();
        if (checkTxSender())
        {
            TESMA_Tx_Counter++;
        }
        if (checkRxGetter())
        {
            TESMA_Rx_Counter++;
            receiveExactoDataStorage();
        }
    }
    else
    {
        if (TESMA_MlineSpiEnableMarker)
        {
            TESMA_MlineSpiEnableMarker = 0;
            turnOffSPI2_FULL_DMA();
        }
    } 
}

int main(int argc, char *argv[]) {
    uint16_t work_freq = 100;
    if (argc > 1)
    {
        work_freq = atoi(argv[1]); 
        printf("value: %d", work_freq);
    }
    else
    {
    }
#ifdef SPI_TXRX_PRINT_ON
    printf("Start Full Duplex SPI\n");
#endif
    resetExactoDataStorage();
    initThreadExactoDataStorage(&TESMA_MainThread);
    lthread_init(&TESMA_DownloadData_Lthread, runTESMA_DownloadData_Lthread);

#ifdef SPI_TXRX_PRINT_ON
    printf("Run cycle for checking:\n");
#endif
//=================================================================================================
   switch (work_freq)
   {
   case 100:
        ex_setFreqHz_SnsService(100, EX_SNSS_ONLY_ACC);
       break;
   case 200:
        ex_setFreqHz_SnsService(200, EX_SNSS_ONLY_ACC);
       break;
   default:
       break;
   } 
//=================================================================================================
    if (ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTESMA_TimReceiver_Lthread))
        return 1;



#ifdef SPI_TXRX_PRINT_ON
    printf("Starting observing of sensor data:");
    printf("\nReceived buffer:\n\n\n ");
#endif
    while(1)
    {
        while(!TESMA_DownloadData_Marker)
        {}
        printBufferData();
        TESMA_DownloadData_Marker = 0;
    }

    return 0;
}

