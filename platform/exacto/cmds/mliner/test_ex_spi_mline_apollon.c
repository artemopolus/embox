#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_tools.h"
#include "spi/spi_mliner.h"
#include "gpio/gpio.h"
#include "tim/tim.h"

#include "sensor/sns_service.h"
#include "kernel/printk.h"

#define SPI_TXRX_PRINT_ON

#define MAX_CALL_COUNT 10

uint8_t TESMA_Sender_Counter = 0;
uint8_t TESMA_Sender_Max = 0;

uint32_t TESMA_Tx_Counter = 0;
uint32_t TESMA_Tx_Buffer;
uint32_t TESMA_Rx_Counter = 0;
uint32_t TESMA_Rx_Buffer;

uint32_t TESMA_Tim_Counter = 0;
uint32_t TESMA_Tim_Buffer;


uint8_t TESMA_MlineSpiEnableMarker = 0;

thread_control_t TESMA_MainThread;


// #define TESMA_DATA_MESSAGE_SIZE SPI_MLINER_BUFFER_SIZE
#define TESMA_DATA_MESSAGE_SIZE 12
uint8_t TESMA_ReceivedData[TESMA_DATA_MESSAGE_SIZE] = { 0};

static struct lthread TESMA_DownloadData_Lthread;
uint8_t               TESMA_DownloadData_Marker = 0;
uint8_t               TESMA_DownloadData_Counter = 0;
uint8_t               TESMA_DownloadData_Max = 9;

uint8_t TESMA_Sync_Marker = 1;

static struct lthread TESMA_ChangeMode_Lthread;

static uint8_t TESMA_print_OutOverFlw_Marker = 0;
static uint32_t TESMA_print_OutOverFlw_Value = 0;

void executeSpiTxRxStage();
static int runTESMA_ChangeMode_Lthread(struct lthread * self)
{
    ex_setFreqHz(400);
    ex_switchStage_SnsService   (EXACTOLINK_LSM303AH_TYPE0);
    ex_setExactolinkType        (EXACTOLINK_LSM303AH_TYPE0);
    TESMA_DownloadData_Marker = 0;
    TESMA_DownloadData_Counter = 0;
    TESMA_DownloadData_Max = 39;
    return 0;
}
static int runTESMA_GpioReceiver_Lthread(struct lthread * self)
{
    if (TESMA_Sync_Marker)
    {
       ex_frcTimReload(); 
       TESMA_Sync_Marker = 0;
    }
    return 0;
}

static int runTESMA_TimReceiver_Lthread(struct lthread * self)
{
    // printk("+");

    if (TESMA_Sender_Counter < TESMA_Sender_Max)
    {
        TESMA_Sender_Counter++;
    }
    else
    {
        TESMA_Tim_Counter++;
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
    uint16_t length = TESMA_DATA_MESSAGE_SIZE;
    uint8_t start_point = 4;
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
    printf("\n");
    // start_point = TESMA_ReceivedData[EXACTOLINK_START_DATA_POINT_ADR];
    start_point = 0;

    printf("\nData to transmit: %d\n", length);
    // for (uint8_t i = start_point  ; i < length; i+=2)
    // {
    //     int16_t value;
    //     ex_convertUint8ToInt16(&TESMA_ReceivedData[i], &value);
    //     printf("%d\t", value);
    // }
    int16_t x, y, z;
    uint16_t buffer_length = ex_getLength_ExDtStr(THR_SPI_TX);
    ex_convertUint8ToInt16(&TESMA_ReceivedData[start_point], &x);
    ex_convertUint8ToInt16(&TESMA_ReceivedData[start_point + 2], &y);
    ex_convertUint8ToInt16(&TESMA_ReceivedData[start_point + 4], &z);

    printf("%d\t%d\t%d|%d %d|%d %d|%d %d|\n", x, y, z,
                                TESMA_ReceivedData[start_point], TESMA_ReceivedData[start_point + 1],
                                TESMA_ReceivedData[start_point+2], TESMA_ReceivedData[start_point + 3],
                                TESMA_ReceivedData[start_point+4], TESMA_ReceivedData[start_point + 5]
                                 );
    printf("Spi info: On: %d| Tx: %d| Rx: %d| Tim: %d | Buf: %d",
                        TESMA_MlineSpiEnableMarker, 
                        TESMA_Tx_Buffer, 
                        TESMA_Rx_Buffer, 
                        TESMA_Tim_Buffer,
                        buffer_length);
    if (TESMA_print_OutOverFlw_Marker)
    {
        TESMA_print_OutOverFlw_Marker = 0;
        printf("| ovr: %d", TESMA_print_OutOverFlw_Value);
    }
    printf("\n");
#endif
    return; 
}

static int runTESMA_DownloadData_Lthread(struct lthread * self)
{
    if(!TESMA_DownloadData_Marker)
    {
        // getMailFromExactoDataStorage(TESMA_ReceivedData, TESMA_DATA_MESSAGE_SIZE);
        if (TESMA_print_OutOverFlw_Marker == 0)
        {
            TESMA_print_OutOverFlw_Marker = 1;
            TESMA_print_OutOverFlw_Value = ExDtStr_OutputSPI_OverFlw;
        }
        watchPackFromExactoDataStorage(TESMA_ReceivedData, 8, 0);
        TESMA_DownloadData_Marker = 1;
        TESMA_Rx_Buffer = ex_getCounter_ExDtStr(THR_SPI_RX);
        TESMA_Tx_Buffer = ex_getCounter_ExDtStr(THR_SPI_TX);
        TESMA_Tim_Buffer = TESMA_Tim_Counter;
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
    transmitExactoDataStorage();

    if (!ex_checkGpio(EX_GPIO_SPI_MLINE))
    {
        if (!TESMA_MlineSpiEnableMarker)
        {
            TESMA_MlineSpiEnableMarker = 1;
            setupSPI2_FULL_DMA();
        }
        if (checkTxSender())
            TESMA_Tx_Counter++;
        if (checkRxGetter())
            TESMA_Rx_Counter++;
    }
    else
    {
        if (TESMA_MlineSpiEnableMarker)
        {
            TESMA_MlineSpiEnableMarker = 0;
            turnOffSPI2_FULL_DMA();
        }
    } 
    // printk("-");
}

int main(int argc, char *argv[]) {

#ifdef SPI_TXRX_PRINT_ON
    printf("Start Full Duplex SPI\n");
#endif
    resetExactoDataStorage();
    initThreadExactoDataStorage(&TESMA_MainThread);
    lthread_init(&TESMA_DownloadData_Lthread, runTESMA_DownloadData_Lthread);
    lthread_init(&TESMA_ChangeMode_Lthread, runTESMA_ChangeMode_Lthread);

#ifdef SPI_TXRX_PRINT_ON
    printf("Run cycle for checking:\n");
#endif
//=================================================================================================


//=================================================================================================
    if (ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTESMA_TimReceiver_Lthread))
        return 1;
    if (ex_subscribeOnGpioEvent(EX_GPIO_SPI_MLINE, runTESMA_GpioReceiver_Lthread))
        return 1;

    lthread_launch(&TESMA_ChangeMode_Lthread);

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
        usleep(100);
    }

    return 0;
}

