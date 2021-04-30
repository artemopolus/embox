#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
  
#include "commander/exacto_data_storage.h"
#include "commander/exacto_filemanager.h"
#include "spi/spi1.h"
#include "gpio/gpio.h"

#define SPI_DMA_TIM_MAX_CALL_COUNT 10
#define SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE 64

uint8_t SpiDmaTimCallIndex = 0;


uint8_t SpiDmaTimDataToBuffer[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1] = {0};
uint8_t SpiDmaTimReceivedData[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1] = { 0};



uint8_t SpiDmaTimCounter = 0;
uint8_t SpiDmaTimMarkerSubscribe = 0;
uint8_t SpiDmaTimLedControl_isenabled = 0;

struct lthread SpiDmaTimSubscribeThread;

struct lthread SpiDmaTimCheckExactoStorageThread;

struct lthread SpiDmaTimLedControlThread;

struct lthread SpiDmaTimSaveToSdThread;

struct lthread SpiDmaTimPrinterWindowThread;

static int runSpiDmaTimPrinterWindowThread(struct lthread * self)
{
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("\033[A\33[2K\r");
    printf("Basic counter: %d\n", SpiDmaTimCounter);
    printf("exactolink\n");
    printf("Received buffer: ");
    for (uint8_t i = 0; i < (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1); i++)
    {
        printf("%d ", SpiDmaTimReceivedData[i]);
    }
    printf("\n");
 
    return 0;
}

static int runSpiDmaTimSaveToSdThread(struct lthread * self)
{
    // ex_saveToFile(SpiDmaTimReceivedData, (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1));
    // uint8_t buffer1[] = "type typ type\n";
    // ex_saveToFile(buffer1, sizeof(buffer1));
    // ex_writeToLogChar("test test test\n");
    return 0;
}

static int runSpiDmaTimLedControlThread(struct lthread * self)
{
    if (SpiDmaTimLedControl_isenabled)
        ex_enableLed(EX_LED_BLUE);
    else
    {
        ex_disableLed(EX_LED_BLUE);
    }
    return 0;
}

static int runSpiDmaTimCheckExactoStorageThread(struct lthread * self)
{
    disableMasterSpiDma();
    ex_disableGpio();
     ex_enableGpio();
    enableMasterSpiDma(); 

   setDataToExactoDataStorage(SpiDmaTimDataToBuffer, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE , THR_CTRL_OK); 
    transmitExactoDataStorage();
    receiveExactoDataStorage();
    getDataFromExactoDataStorage(SpiDmaTimReceivedData, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE );
    return 0;
}

static int runPrintReceivedData(struct  lthread * self)
{
    SpiDmaTimCounter++;
    

    if (SpiDmaTimCallIndex < SPI_DMA_TIM_MAX_CALL_COUNT)
    {
        SpiDmaTimCallIndex++;
    }
    else{
    if (SpiDmaTimReceivedData[0] == EXACTOLINK_PCK_ID)
    {
        // printf("\033[A\33[2K\r");
        // printf("\033[A\33[2K\r");
        // printf("\033[A\33[2K\r");
        // printf("Basic counter: %d\n", SpiDmaTimCounter);
        // printf("exactolink\n");
        // printf("Received buffer: ");
        // for (uint8_t i = 0; i < (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1); i++)
        // {
        //     printf("%d ", SpiDmaTimReceivedData[i]);
        // }
        // // ex_saveToFile(SpiDmaTimReceivedData, (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1));
        // printf("\n");
        lthread_launch(&SpiDmaTimPrinterWindowThread);
        lthread_launch(&SpiDmaTimSaveToSdThread);
        if (!SpiDmaTimLedControl_isenabled)
        {
            SpiDmaTimLedControl_isenabled = 1;
            lthread_launch(&SpiDmaTimLedControlThread);
        }
    }
        else
        {
            printf("\033[A\33[2K\r");
            printf("\033[A\33[2K\r");
            printf("\033[A\33[2K\r");
            printf("Basic counter: %d\n", SpiDmaTimCounter);
            printf("Unknown package\n");
            for (uint8_t i = 0; i < (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1); i++)
            {
                printf("%d ", SpiDmaTimReceivedData[i]);
            }
            printf("\n");
        }  
        lthread_launch(&SpiDmaTimCheckExactoStorageThread);
        SpiDmaTimCallIndex = 0;
    }
    return 0;
}
static int runSpiDmaTimSubcribeThread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runPrintReceivedData);
    if (result == 0)
        SpiDmaTimMarkerSubscribe = 1;
    return 0;
}

int main(int argc, char *argv[]) {
    SpiDmaTimReceivedData[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE] = '\n';
    initExactoFileManager();
    ex_enableLed(EX_LED_GREEN);

    printf("Start print function\n");

    lthread_init(&SpiDmaTimSubscribeThread, runSpiDmaTimSubcribeThread);
    lthread_init(&SpiDmaTimCheckExactoStorageThread, runSpiDmaTimCheckExactoStorageThread);
    lthread_init(&SpiDmaTimLedControlThread, runSpiDmaTimLedControlThread);

    lthread_init(&SpiDmaTimSaveToSdThread, runSpiDmaTimSaveToSdThread);
    lthread_init(&SpiDmaTimPrinterWindowThread, runSpiDmaTimPrinterWindowThread);

    lthread_launch(&SpiDmaTimSubscribeThread);

    printf("Wait subscribing...\n\n\n\n\n");
    ex_writeToLogChar("Start waiting data from sensors\n");

    while (!SpiDmaTimMarkerSubscribe)
    {
    }
    // printf("Start Full Duplex SPI\n");

    while(1)
    {
        usleep(100000);
        ex_writeToLogChar("Ping\n");
        uint8_t buffer1[] = "type typ type\n";
        ex_saveToFile(buffer1, sizeof(buffer1));

    }
    return 0;
}