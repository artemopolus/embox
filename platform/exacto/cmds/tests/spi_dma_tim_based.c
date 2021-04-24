#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
  
#include "commander/exacto_data_storage.h"
#include "spi/spi1.h"
#include "gpio/gpio.h"

#define SPI_DMA_TIM_MAX_CALL_COUNT 10
#define SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE 64

uint8_t SpiDmaTimCallIndex = 0;


uint8_t SpiDmaTimDataToBuffer[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE] = {0};
uint8_t SpiDmaTimReceivedData[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE] = { 0};



uint8_t SpiDmaTimCounter = 0;
uint8_t SpiDmaTimMarkerSubscribe = 0;

struct lthread SpiDmaTimSubscribeThread;

struct lthread SpiDmaTimCheckExactoStorageThread;

static int runSpiDmaTimCheckExactoStorageThread(struct lthread * self)
{
    disableMasterSpiDma();
    ex_disableGpio();
     ex_enableGpio();
    enableMasterSpiDma(); 

   setDataToExactoDataStorage(SpiDmaTimDataToBuffer, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE, THR_CTRL_OK); 
    transmitExactoDataStorage();
    receiveExactoDataStorage();
    getDataFromExactoDataStorage(SpiDmaTimReceivedData, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE);
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
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("Basic counter: %d\n", SpiDmaTimCounter);
        printf("exactolink\n");
        printf("Received buffer: ");
        for (uint8_t i = 0; i < SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE; i++)
        {
            printf("%d ", SpiDmaTimReceivedData[i]);
        }
        printf("\n");
    }
    else
    {
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("Basic counter: %d\n", SpiDmaTimCounter);
        printf("Unknown package\n");
        for (uint8_t i = 0; i < SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE; i++)
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
    printf("Start print function\n");

    lthread_init(&SpiDmaTimSubscribeThread, runSpiDmaTimSubcribeThread);
    lthread_init(&SpiDmaTimCheckExactoStorageThread, runSpiDmaTimCheckExactoStorageThread);
    lthread_launch(&SpiDmaTimSubscribeThread);

    printf("Wait subscribing...\n\n\n\n\n");

    while (!SpiDmaTimMarkerSubscribe)
    {
    }
    // printf("Start Full Duplex SPI\n");

    while(1)
    {
    }
    return 0;
}