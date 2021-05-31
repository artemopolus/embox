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
#include "sensors/exacto_datatools.h"
#include "spi/spi_mliner.h"
#include "gpio/gpio.h"

static struct lthread TESP_Subscribe_Lthread;
uint8_t               TESP_Subscribe_Marker = 0;

static struct thread *  TESP_WindowPrinter_Thread;
static cond_t           TESP_WindowPrinter_Signal;
static struct mutex     TESP_WindowPrinter_Mutex;
static struct lthread   TESP_WindowPrinter_Remainder_Lthread;
uint8_t                 TESP_WindowPrinter_Counter = 0;
uint8_t                 TESP_WindowPrinter_Max = 9;
uint8_t               TESMAF_WindowPrinter_Marker = 0;

uint32_t TESP_TimReceiver_Counter = 0;
uint32_t TESP_TimReceiver_Buffer = 0;

static struct thread *  TESP_PrintToSD_Thread;
static cond_t           TESP_PrintToSD_Signal;
static struct mutex     TESP_PrintToSD_Mutex;
static struct lthread   TESP_PrintToSD_Remainder_Lthread;

//----------------------------------------------------------------------------
#define TESMAF_MESSAGE_SIZE SPI_MLINER_BUFFER_SIZE
uint8_t TESMAF_DataToBuffer[TESMAF_MESSAGE_SIZE] = {0};
uint8_t TESMAF_ReceivedData[TESMAF_MESSAGE_SIZE] = {0};

uint8_t TESMAF_CheckDiv_Counter = 0;
uint8_t TESMAF_CheckDiv_Max = 9;
static struct lthread   TESMAF_CheckExactoStorage_Lthread;
static struct mutex     TESMAF_CheckExactoStorage_Mutex; 

#define TESMAF_RECEIVED_DATA_SZ 6
int16_t                     TESMAF_ReceivedData_Data[TESMAF_RECEIVED_DATA_SZ] = {0};
exactolink_package_info_t   TESMAF_ReceivedData_Info;
uint32_t TESMAF_Tx_Buffer;
uint32_t TESMAF_Rx_Buffer;
uint32_t TESMAF_DataCheck_Counter = 0;
uint32_t TESMAF_DataCheck_Success = 0;
uint32_t TESMAF_DataCheck_CntBuff;
uint32_t TESMAF_DataCheck_ScsBuff;

uint8_t TESMAF_Sync_Marker = 1;

uint8_t TESMAF_Sensors_Marker = 0;
//проверка 
static uint8_t TESMAF_Sensors_TickCnt = 0;
static uint8_t TESMAF_Sensors_TickMax = 200;
static uint8_t TESMAF_Sensors_BadCnt = 0;
static uint8_t TESMAF_Sensors_BadMax = 20;

static int runTESMAF_CheckExactoStorage_Lthread(struct lthread * self)
{
    start:
    // printk("&");
    if (TESMAF_Sync_Marker)
    {
        ex_disableGpio(EX_GPIO_SPI_SYNC);
        TESMAF_Sync_Marker = 0;
    }
    
    disableMasterSpiDma();
    ex_enableGpio(EX_GPIO_SPI_MLINE);
    ex_disableGpio(EX_GPIO_SPI_MLINE);
    enableMasterSpiDma();
    TESMAF_DataCheck_Counter++; 

    setDataToExactoDataStorage(TESMAF_DataToBuffer, TESMAF_MESSAGE_SIZE , THR_CTRL_OK); 
    transmitExactoDataStorage();
    receiveExactoDataStorage();
    mutex_retry:
	if (mutex_trylock_lthread(self, &TESMAF_CheckExactoStorage_Mutex) == -EAGAIN) {
        return lthread_yield(&&start, &&mutex_retry);
	}
    // getDataFromExactoDataStorage(TESMAF_ReceivedData, TESMAF_MESSAGE_SIZE );
    if (TESMAF_Sensors_TickCnt < TESMAF_Sensors_TickMax)
    {
        TESMAF_Sensors_TickCnt++;
    }
    else
    {
        if (TESMAF_Sensors_BadCnt > TESMAF_Sensors_BadMax)
        {
            //Датчики слишком долго ничего не слали
            if (TESMAF_Sensors_Marker)
            {
                TESMAF_Sensors_Marker = 0;
                ex_disableLed(EX_LED_BLUE);
            }
        }
        TESMAF_Sensors_TickCnt = 0;
        TESMAF_Sensors_BadCnt = 0;
    }
    if (ex_checkData_ExDtStr() == EXACTOLINK_LSM303AH_TYPE0)
    {
        if (!TESMAF_Sensors_Marker)
        {
            ex_enableLed(EX_LED_BLUE);
            TESMAF_Sensors_Marker = 1;
        }
        ex_getInfo_ExDtStr(&TESMAF_ReceivedData_Info);
        ex_getData_ExDtStr(TESMAF_ReceivedData, 12, THR_SPI_RX);
        TESMAF_Rx_Buffer = ex_getCounter_ExDtStr(THR_SPI_RX);
        TESMAF_Tx_Buffer = ex_getCounter_ExDtStr(THR_SPI_TX);
        TESMAF_DataCheck_Success++;
        TESMAF_DataCheck_CntBuff = TESMAF_DataCheck_Counter;
        TESMAF_DataCheck_ScsBuff = TESMAF_DataCheck_Success;
        
    }
    else
    {
        TESMAF_Sensors_BadCnt++;
    }
    if (TESMAF_WindowPrinter_Marker == 1)
    {
        if (TESMAF_ReceivedData_Info.packagetype == EXACTOLINK_LSM303AH_TYPE0)
        {
            for (uint8_t i = 0; i < 6; i++)
            {
                ex_convertUint8ToInt16(&TESMAF_ReceivedData[2*i], &TESMAF_ReceivedData_Data[i]);
            }
            TESMAF_WindowPrinter_Marker = 2;
            // lthread_launch(&TESP_WindowPrinter_Remainder_Lthread);
        }
    }
	mutex_unlock_lthread(self, &TESMAF_CheckExactoStorage_Mutex);
    return 0;
}
void updateMline()
{
    if (TESMAF_CheckDiv_Counter < TESMAF_CheckDiv_Max)
    {
        TESMAF_CheckDiv_Counter++;
    }
    else
    {
        lthread_launch(&TESMAF_CheckExactoStorage_Lthread);
        TESMAF_CheckDiv_Counter = 0;
        if (TESMAF_ReceivedData[0] == EXACTOLINK_PCK_ID)
        {
            uint64_t counter = 0;
            ex_convertUint8ToUint64(&TESMAF_ReceivedData[0], &counter);
            lthread_launch(&TESP_PrintToSD_Remainder_Lthread);
        }
    }
}
//----------------------------------------------------------------------------

static int runTESP_PrintToSD_Remainder_Lthread(struct lthread * self)
{
	if (mutex_trylock_lthread(self, &TESP_PrintToSD_Mutex) == -EAGAIN) {
        return 0;
	}
    cond_signal(&TESP_PrintToSD_Signal);
	mutex_unlock_lthread(self, &TESP_PrintToSD_Mutex);
    return 0;
}
static void * runTESP_PrintToSD_Thread(void * arg)
{
    // uint8_t test_string[] = "Data from afina\n";

    while(1)
    {
        mutex_lock(&TESMAF_CheckExactoStorage_Mutex);
        // ex_saveToFile(test_string, sizeof(test_string));
        ex_saveToFile(TESMAF_ReceivedData, TESMAF_MESSAGE_SIZE);
        mutex_unlock(&TESMAF_CheckExactoStorage_Mutex);
        mutex_lock(&TESP_PrintToSD_Mutex);
        cond_wait(&TESP_PrintToSD_Signal, &TESP_PrintToSD_Mutex);
        mutex_unlock(&TESP_PrintToSD_Mutex);
    }    
    return NULL;
}
static int runTESP_WindowPrinter_Remainder_Lthread(struct lthread * self)
{
    // start:
    // mutex_retry:
	if (mutex_trylock_lthread(self, &TESP_WindowPrinter_Mutex) == -EAGAIN) {
        // return lthread_yield(&&start, &&mutex_retry);
        return 0;
	}
    // printk("after mutex\n");
    TESP_TimReceiver_Buffer = TESP_TimReceiver_Counter;
    cond_signal(&TESP_WindowPrinter_Signal);
	mutex_unlock_lthread(self, &TESP_WindowPrinter_Mutex);
    return 0;
}
static void * runTESP_WindowPrinter_Thread(void * arg)
{
    printf("Start reporter!\n\n\n");
    while(1)
    {
        mutex_lock(&TESP_WindowPrinter_Mutex);
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("Data received:\n");
        for (int i = 0; i < TESMAF_RECEIVED_DATA_SZ; i++)
        {
            printf("%d\t",TESMAF_ReceivedData_Data[i]);
        }
        printf("\nRefCnt: %d| Tx: %d| Rx: %d| Chck: %d| Scs: %d\n", TESMAF_ReceivedData_Info.counter,
                     TESMAF_Rx_Buffer, TESMAF_Tx_Buffer, TESMAF_DataCheck_CntBuff, TESMAF_DataCheck_ScsBuff);

        TESMAF_WindowPrinter_Marker = 0;
        cond_wait(&TESP_WindowPrinter_Signal, &TESP_WindowPrinter_Mutex);
        mutex_unlock(&TESP_WindowPrinter_Mutex);
    }
    return NULL;
}
static int runTESP_TimReceiver_Lthread(struct  lthread * self)
{
    TESP_TimReceiver_Counter++;
    // printk("+");
    // lthread_launch(&TESP_PrintToSD_Remainder_Lthread);
    updateMline();
    if (TESP_WindowPrinter_Counter < TESP_WindowPrinter_Max)
    {
        TESP_WindowPrinter_Counter++;
    }
    else
    {
        TESP_WindowPrinter_Counter = 0;
        // printk("i\n");
        lthread_launch(&TESP_WindowPrinter_Remainder_Lthread);
        if (TESMAF_WindowPrinter_Marker == 0)
            TESMAF_WindowPrinter_Marker = 1;
    }
    return 0;
}
static int runTESP_Subscribe_Lthread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTESP_TimReceiver_Lthread);
    if (result == 0)
        TESP_Subscribe_Marker = 1;
    return 0;
}
int main(int argc, char *argv[]) {
    if(initExactoFileManager())
    {
        printf("Can't run SD card\n");
        return 0;
    }

    printf("Create file\nWrite in file: Data from afina\nReporting about execution\n\n\n");

    lthread_init(&TESP_Subscribe_Lthread, &runTESP_Subscribe_Lthread);
    lthread_init(&TESP_WindowPrinter_Remainder_Lthread, &runTESP_WindowPrinter_Remainder_Lthread);

    TESP_WindowPrinter_Thread = thread_create(THREAD_FLAG_SUSPENDED, runTESP_WindowPrinter_Thread, NULL);
    mutex_init(&TESP_WindowPrinter_Mutex);
    cond_init(&TESP_WindowPrinter_Signal, NULL);
    lthread_init(&TESP_WindowPrinter_Remainder_Lthread, runTESP_WindowPrinter_Remainder_Lthread);

    TESP_PrintToSD_Thread = thread_create(THREAD_FLAG_SUSPENDED, runTESP_PrintToSD_Thread, NULL);
    mutex_init(&TESP_PrintToSD_Mutex);
    cond_init(&TESP_PrintToSD_Signal, NULL);
    lthread_init(&TESP_PrintToSD_Remainder_Lthread, runTESP_PrintToSD_Remainder_Lthread);

    lthread_init(&TESMAF_CheckExactoStorage_Lthread, runTESMAF_CheckExactoStorage_Lthread);
    
    
    schedee_priority_set(&TESP_PrintToSD_Thread->schedee, 200);
    schedee_priority_set(&TESP_PrintToSD_Remainder_Lthread.schedee, 230);
    schedee_priority_set(&TESP_WindowPrinter_Thread->schedee, 150);
    schedee_priority_set(&TESP_WindowPrinter_Remainder_Lthread.schedee, 210);

    lthread_launch(&TESP_Subscribe_Lthread);
    while(!TESP_Subscribe_Marker)
    {
    }
    printf("Subscribing is done\n");

    printk("+ -- tim run\ni -- win run\n& -- exdtst run\n");

    thread_launch(TESP_PrintToSD_Thread);

    thread_launch(TESP_WindowPrinter_Thread);
    thread_join(TESP_PrintToSD_Thread,NULL);
    thread_join(TESP_WindowPrinter_Thread, NULL);

    return 0;
}
