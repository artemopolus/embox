#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/thread.h>
#include <kernel/thread/sync/mutex.h>
#include <kernel/thread/sync/cond.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/task.h>
#include <kernel/time/ktime.h>
#include <kernel/printk.h>

#include "tim/tim.h"
  
#include "commander/exacto_data_storage.h"
#include "commander/exacto_filemanager.h"
// #include "sensors/exacto_datatools.h"
#include "spi/spi_mliner.h"
#include "gpio/gpio.h"

#define SPI_DMA_TIM_MAX_CALL_COUNT 10
#define SPI_DMA_TIM_SPI_CHECKER_DIVIDER 10
#define SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE 64

uint8_t SpiDmaTimCallIndex = 0;


uint8_t SpiDmaTimDataToBuffer[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1] = {0};
uint8_t SpiDmaTimReceivedData[SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1] = { 0};



uint8_t SpiDmaTimCounter = 0;
uint8_t SpiDmaTimCheckDivCounter = 0;
uint8_t SpiDmaTimMarkerSubscribe = 0;
uint8_t SpiDmaTimLedControl_isenabled = 0;

struct lthread SpiDmaTimSubscribeThread;

struct lthread SpiDmaTimCheckExactoStorageThread;

struct lthread SpiDmaTimLedControlThread;

struct lthread SpiDmaTimSaveToSdThread;

struct lthread SpiDmaTimPrinterWindowThread;


static struct thread *SpiDmaTim_PrintToSD_Thread;
static cond_t SpiDmaTim_SignalForSD;
static struct mutex SpiDmaTim_MutexForSD;

static struct thread *  SpiDmaTim_WindowPrinter_Thread;
static cond_t           SpiDmaTim_WindowPrinter_Signal;
static struct mutex     SpiDmaTim_WindowPrinter_Mutex;
struct lthread          SpiDmaTim_WinPrintRemainder_Lthread;
 
static void * runSpiDmaTim_WindowPrinter_Thread(void * arg)
{
    while(1)
    {
        if (SpiDmaTimReceivedData[0] == EXACTOLINK_PCK_ID)
        {
            printf("\033[A\33[2K\r");
            printf("\033[A\33[2K\r");
            // printf("\033[A\33[2K\r");
            // printf("\033[A\33[2K\r");
            uint64_t counter = 0;
            uint8_t start_point = SpiDmaTimReceivedData[EXACTOLINK_START_DATA_POINT_ADR];
            ex_convertUint8ToUint64(&SpiDmaTimReceivedData[start_point], &counter);
            printf("Basic counter: %d Received counter: %d\n", SpiDmaTimCounter, counter);
            // printf("exactolink\n");
            printf("Received buffer(exactolink): ");
            for (uint8_t i = 0; i < (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1); i++)
            {
                printf("%d ", SpiDmaTimReceivedData[i]);
            }
            printf("\n");
        }
            // printf("\033[A\33[2K\r");
            // printf("\033[A\33[2K\r");
            // printf("\033[A\33[2K\r");
            // printf("Basic counter: %d\n", SpiDmaTimCounter);
            // printf("Unknown package\n");
            // for (uint8_t i = 0; i < (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE + 1); i++)
            // {
            //     printf("%d ", SpiDmaTimReceivedData[i]);
            // }
            // printf("\n");
        //-------------------------------------------------------------------------
        mutex_lock(&SpiDmaTim_WindowPrinter_Mutex);
        cond_wait(&SpiDmaTim_WindowPrinter_Signal, &SpiDmaTim_WindowPrinter_Mutex);
        mutex_unlock(&SpiDmaTim_WindowPrinter_Mutex);
    }
    return NULL;
}
static void *runSpiDmaTim_PrintToSD_Thread(void *arg) {
    printk("Run printer to sd card");
    while(1)
    {
        // printf("Try to save to file\n");
        ex_saveToFile(SpiDmaTimReceivedData, (SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE+1));
        mutex_lock(&SpiDmaTim_MutexForSD);
        cond_wait(&SpiDmaTim_SignalForSD, &SpiDmaTim_MutexForSD);
        mutex_unlock(&SpiDmaTim_MutexForSD);
    }    
    return NULL;
}
static int runSpiDmaTim_WinPrintRemainder_Lthread(struct lthread * self)
{
    // start:
    // mutex_retry:
    // printk("Light thread run\n");
	if (mutex_trylock_lthread(self, &SpiDmaTim_WindowPrinter_Mutex) == -EAGAIN) {
        // return lthread_yield(&&start, &&mutex_retry);
        return 0;
	}
    // printk("after mutex\n");
    cond_signal(&SpiDmaTim_WindowPrinter_Signal);
	mutex_unlock_lthread(self, &SpiDmaTim_WindowPrinter_Mutex);
    return 0;
}

static int runSpiDmaTimSaveToSdThread(struct lthread * self)
{
    start:
    mutex_retry:
    // printk("Light thread run\n");
	if (mutex_trylock_lthread(self, &SpiDmaTim_MutexForSD) == -EAGAIN) {
        return lthread_yield(&&start, &&mutex_retry);
        // return 0;
	}
    // printk("after mutex\n");
    cond_signal(&SpiDmaTim_SignalForSD);
	mutex_unlock_lthread(self, &SpiDmaTim_MutexForSD);

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
    ex_disableGpio(EX_GPIO_SPI_MLINE);
    ex_enableGpio(EX_GPIO_SPI_MLINE);
    enableMasterSpiDma(); 

    setDataToExactoDataStorage(SpiDmaTimDataToBuffer, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE , EX_THR_CTRL_OK); 
    transmitExactoDataStorage();
    receiveExactoDataStorage();
    getDataFromExactoDataStorage(SpiDmaTimReceivedData, SPI_DMA_TIM_TRANSMIT_MESSAGE_SIZE );
    return 0;
}

static int runPrintReceivedData(struct  lthread * self)
{
    SpiDmaTimCounter++;

    if (SpiDmaTimCheckDivCounter < SPI_DMA_TIM_SPI_CHECKER_DIVIDER)
    {
        SpiDmaTimCheckDivCounter++;
    }
    else
    {
        lthread_launch(&SpiDmaTimCheckExactoStorageThread);
        SpiDmaTimCheckDivCounter = 0;
        if (SpiDmaTimReceivedData[0] == EXACTOLINK_PCK_ID)
        {
            uint64_t counter = 0;
            ex_convertUint8ToUint64(&SpiDmaTimReceivedData[0], &counter);
            lthread_launch(&SpiDmaTimSaveToSdThread);
        }
    }
 

    if (SpiDmaTimCallIndex < SPI_DMA_TIM_MAX_CALL_COUNT)
    {
        SpiDmaTimCallIndex++;
    }
    else
    {
        if (SpiDmaTimReceivedData[0] == EXACTOLINK_PCK_ID)
        {
            lthread_launch(&SpiDmaTimPrinterWindowThread);
            if (!SpiDmaTimLedControl_isenabled)
            {
                SpiDmaTimLedControl_isenabled = 1;
                lthread_launch(&SpiDmaTimLedControlThread);
            }
        }
        lthread_launch(&SpiDmaTim_WinPrintRemainder_Lthread);
       SpiDmaTimCallIndex = 0;
    }
    return 0;
}
static int runSpiDmaTimSubcribeThread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runPrintReceivedData);
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

	SpiDmaTim_PrintToSD_Thread = thread_create(THREAD_FLAG_SUSPENDED, runSpiDmaTim_PrintToSD_Thread, NULL);
	mutex_init(&SpiDmaTim_MutexForSD);
	cond_init(&SpiDmaTim_SignalForSD, NULL);

    SpiDmaTim_WindowPrinter_Thread = thread_create(THREAD_FLAG_SUSPENDED, runSpiDmaTim_WindowPrinter_Thread, NULL);
    mutex_init(&SpiDmaTim_WindowPrinter_Mutex);
    cond_init(&SpiDmaTim_WindowPrinter_Signal, NULL);
    lthread_init(&SpiDmaTim_WinPrintRemainder_Lthread, runSpiDmaTim_WinPrintRemainder_Lthread);
    
    schedee_priority_set(&SpiDmaTim_PrintToSD_Thread->schedee, 200);
    schedee_priority_set(&SpiDmaTimSaveToSdThread.schedee,210);
    schedee_priority_set(&SpiDmaTim_WindowPrinter_Thread->schedee, 150);

    lthread_launch(&SpiDmaTimSubscribeThread);

    printf("Wait subscribing...\n\n\n\n\n");
    ex_writeToLogChar("Start waiting data from sensors\n");

    while (!SpiDmaTimMarkerSubscribe)
    {
    }
    // printf("Start Full Duplex SPI\n");
    thread_launch(SpiDmaTim_PrintToSD_Thread);
    thread_join(SpiDmaTim_PrintToSD_Thread, NULL);

    thread_launch(SpiDmaTim_WindowPrinter_Thread);
    thread_join(SpiDmaTim_WindowPrinter_Thread, NULL);


    while(1)
    {
        // usleep(100000);
        // ex_writeToLogChar("Ping\n");
        // uint8_t buffer1[] = "type typ type\n";
        // ex_saveToFile(buffer1, sizeof(buffer1));

    }
    return 0;
}