#include "mliner/main_mod.h"

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
  
#include "commander/exacto_filemanager.h"

#include "spi/spi_mliner.h"
#include "gpio/gpio_spi.h"
#include "gpio/gpio_led.h"

#ifdef TESMAF_TICK_VIZ
#include "ex_utils.h"

static uint32_t     TESMAF_Ticker_Start = 0, 
                    TESMAF_Ticker_Stop = 0, 
                    TESMAF_Ticker_Result = 0;
#endif

static mliner_main_mod_modes_t Mode = MLINER_M_FIRST_START;

static struct lthread TESP_Subscribe_Lthread;
uint8_t               TESP_Subscribe_Marker = 0;

uint32_t TESP_TimReceiver_Counter = 0;
uint32_t TESP_TimReceiver_Buffer = 0;

static struct thread *  TESP_PrintToSD_Thread;
static cond_t           TESP_PrintToSD_Signal;
static struct mutex     TESP_PrintToSD_Mutex;
static struct lthread   TESP_PrintToSD_Remainder_Lthread;
static uint8_t          TESP_PrintToSD_enabled = 0;

static struct lthread GetValue_Lthread;
uint32_t * GetValuePtr;
uint8_t * GetValueRes;
mliner_main_mod_vars_t GetValueType;
//----------------------------------------------------------------------------

#define TESMAF_MESSAGE_SIZE EXACTO_BUFFER_UINT8_SZ
uint8_t TESMAF_DataToBuffer[TESMAF_MESSAGE_SIZE] = {0};
static uint8_t                  TESMAF_ReceivedData_Counter = 0;


uint8_t TESMAF_CheckDiv_Counter = 0;
uint8_t TESMAF_CheckDiv_Max = 2;
static struct lthread   TESMAF_CheckExactoStorage_Lthread;
static struct mutex     TESMAF_CheckExactoStorage_Mutex; 

static uint32_t TESMAF_CounterBuffer_Input = 0;
static uint32_t TESMAF_CounterBuffer_Middl = 0;

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
uint8_t TESMAF_Sensors_TickCnt = 0;
uint8_t TESMAF_Sensors_TickMax = 200;
uint8_t TESMAF_Sensors_GoodCnt = 0;
uint8_t TESMAF_Sensors_GoodMax = 5;

uint8_t TESMAF_test_uploaddatamarker = 0;
uint8_t TESMAF_test_pushtosdmarker = 0;
uint8_t TESMAF_test_PushToSdMarkerGood = 0;
uint8_t TESMAF_test_PushToSdMarkerBad = 0;

uint32_t TESMAF_test_CallFunTooManyFailed = 0;
uint32_t TESMAF_test_InputLst = 0;
uint32_t TESMAF_test_UpdteLst = 0;


static struct lthread   TESMAF_AfterCheckExStr_Lthread;
static int runTESMAF_AfterCheckExStr_Lthread(struct lthread * self)
{
    exactolink_package_result_t exactolink_result;
    uint32_t cnt_buf = 0;
    uint32_t cnt_buf2 = 0;
start:
    exactolink_result = ex_checkData_ExDtStr();
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("^");
#endif
    if   (!((   exactolink_result == EXACTOLINK_LSM303AH_TYPE0) ||
          (     exactolink_result == EXACTOLINK_SNS_XLXLGR)))
    {
        // printk("s");
        //данные не прошли проверку валидности
        return 0;
    }
    cnt_buf = TESMAF_ReceivedData_Info.counter;
    if ((TESMAF_CounterBuffer_Input - TESMAF_CounterBuffer_Middl) > 5)
    {
        //количество вызовов функций не совпадает с ее удачным завершением
        TESMAF_test_CallFunTooManyFailed++;
        // printk("?");
    }
    if ((cnt_buf - TESMAF_CounterBuffer_Input) > 1)
    {
        //пропущены данные, которые отправлены от аполлона
        // printk(">");
        TESMAF_test_InputLst++;
    }    
    TESMAF_CounterBuffer_Input = cnt_buf;

    ex_getInfo_ExDtStr(&TESMAF_ReceivedData_Info);
    TESMAF_ReceivedData_Counter++;
mutex_chk:
	if (mutex_trylock_lthread(self, &TESMAF_CheckExactoStorage_Mutex) == -EAGAIN) {
        return lthread_yield(&&start, &&mutex_chk);
	}
    TESMAF_test_uploaddatamarker = 1;

    cnt_buf2 = TESMAF_ReceivedData_Info.counter;
    if ((cnt_buf2 - TESMAF_CounterBuffer_Middl) > 1)
    {
        // printk("<");
        //были пропущены данные
        TESMAF_test_UpdteLst++;
    }    
    TESMAF_CounterBuffer_Middl = cnt_buf2;

    TESMAF_test_uploaddatamarker = 0;
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("-");
#endif
	mutex_unlock_lthread(self, &TESMAF_CheckExactoStorage_Mutex);
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("+");
#endif
    TESMAF_Rx_Buffer = ex_getCounter_ExDtStr(EX_THR_SPi_RX);
    TESMAF_Tx_Buffer = ex_getCounter_ExDtStr(EX_THR_SPi_TX);
    TESMAF_DataCheck_Success++;
    TESMAF_DataCheck_CntBuff = TESMAF_DataCheck_Counter;
    TESMAF_DataCheck_ScsBuff = TESMAF_DataCheck_Success;
    
    TESMAF_Sensors_GoodCnt++; //<======================================
    lthread_launch(&TESP_PrintToSD_Remainder_Lthread);
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("&");
#endif
    return 0;
}
static int runTESMAF_CheckExactoStorage_Lthread(struct lthread * self)
{
    // printk("&");
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("$");
#endif
    
    TESMAF_DataCheck_Counter++; 

    setDataToExactoDataStorage(TESMAF_DataToBuffer, TESMAF_MESSAGE_SIZE , EX_THR_CTRL_OK);
    if (ExDtStr_TrasmitSPI_DbleCnt > 100)
    {
        // ExDtStr_TrasmitSPI_DbleCnt = 0;
        // syncMasterSpiDma();
    } 
    transmitExactoDataStorage();
    lthread_launch(&TESMAF_AfterCheckExStr_Lthread);


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
    }
}
//----------------------------------------------------------------------------

static int runTESP_PrintToSD_Remainder_Lthread(struct lthread * self)
{
    mutex_retry:
	if (mutex_trylock_lthread(self, &TESP_PrintToSD_Mutex) == -EAGAIN) {
        return lthread_yield(&&mutex_retry, &&mutex_retry);
	}
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("#\n");
#endif
    ex_saveDataToExBufSD();
    cond_signal(&TESP_PrintToSD_Signal);
	mutex_unlock_lthread(self, &TESP_PrintToSD_Mutex);
    return 0;
}
static void * runTESP_PrintToSD_Thread(void * arg)
{
    // uint8_t test_string[] = "Data from afina\n";
    uint32_t data_to_sd_cnt = 0;
    uint32_t lst_cnt = 0;
    uint32_t delta = 0;
    uint16_t fast_write_cnt = 0;

    while (!TESP_PrintToSD_enabled)
       sleep(1);
     
    if(initExactoFileManager())
    {
        return NULL;
    }
    TESP_PrintToSD_enabled = 2;

    while(1)
    {
#ifdef PRINTK_ID_FOR_THREAD_ON
        printk("[");
#endif
        mutex_lock(&TESP_PrintToSD_Mutex);
        mutex_lock(&TESMAF_CheckExactoStorage_Mutex);
        uint32_t current_cnt = TESMAF_CounterBuffer_Middl;
        delta = current_cnt - data_to_sd_cnt;
        if ((delta) > 1)
        {
            // printk("&");
            lst_cnt++;
            fast_write_cnt = 0;
        }
        else if ((current_cnt - data_to_sd_cnt) == 0)
        {
            // printk("*");
        }
        else
        {
            fast_write_cnt++;
        }
        data_to_sd_cnt = current_cnt;
        TESMAF_test_pushtosdmarker = 1;
        TESMAF_ReceivedData_Counter = 0;
        TESMAF_test_pushtosdmarker = 0;
#ifdef PRINTK_ID_FOR_THREAD_ON
        printk("~b");
#endif
        mutex_unlock(&TESMAF_CheckExactoStorage_Mutex);
#ifdef PRINTK_ID_FOR_THREAD_ON
        printk("~c");
#endif
#ifdef TESMAF_TICK_VIZ
        TESMAF_Ticker_Start = ex_dwt_cyccnt_start();
#endif
        if(ex_pshExBufToSD())
        {
            TESMAF_test_PushToSdMarkerBad++;
        }
        else
        {
            TESMAF_test_PushToSdMarkerGood++;
        }
#ifdef TESMAF_TICK_VIZ
        TESMAF_Ticker_Stop = ex_dwt_cyccnt_stop();
#endif
#ifdef PRINTK_ID_FOR_THREAD_ON
        printk("~d");
#endif
        
        // mutex_lock(&TESP_PrintToSD_Mutex);
#ifdef TESMAF_TICK_VIZ
        TESMAF_Ticker_Result = TESMAF_Ticker_Stop - TESMAF_Ticker_Start;
#ifdef PRINTK_ID_FOR_THREAD_ON
        printk("~%d]", TESMAF_Ticker_Result);
#endif
#endif
        cond_wait(&TESP_PrintToSD_Signal, &TESP_PrintToSD_Mutex);
        mutex_unlock(&TESP_PrintToSD_Mutex);
    }    
    return NULL;
}

static int runTESP_TimReceiver_Lthread(struct  lthread * self)
{
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("!");
#endif
	ex_subs_service_t * trg = (ex_subs_service_t*)self;
   // printk("!");
    if (Mode == MLINER_M_STOP_ALL)
        return 0;
    if (TESP_TimReceiver_Counter == 0)
    {
        uint8_t str[] = "Start TIM recv\n";
        exfm_print2log(str, sizeof(str));
    }

        // ex_writeToLogChar("Start TIM recv\n");
    if (TESP_TimReceiver_Counter % 1000);
    {
        // ex_writeToLogChar("1000 done\n");
        uint8_t str[] = "100 done\n";
        exfm_print2log(str, sizeof(str));
    }
    TESP_TimReceiver_Counter++;
    if (TESMAF_Sensors_TickCnt < TESMAF_Sensors_TickMax)
    {
        TESMAF_Sensors_TickCnt++;
    }
    else
    {
        if (TESMAF_Sensors_GoodCnt > TESMAF_Sensors_GoodMax)
        {
            if (!TESMAF_Sensors_Marker)
            {
                ex_enableLed(EX_LED_BLUE);
                TESMAF_Sensors_Marker = 1;
            }
        }
        else
        {
            //Датчики слишком долго ничего не слали
            if (TESMAF_Sensors_Marker)
            {
                TESMAF_Sensors_Marker = 0;
                ex_disableLed(EX_LED_BLUE);
            }
            if (TESMAF_Sync_Marker)
            {
                // ex_disableGpio(EX_GPIO_SPI_SYNC);
                // TESMAF_Sync_Marker = 0;
            }
        }
        TESMAF_Sensors_TickCnt = 0;
        TESMAF_Sensors_GoodCnt = 0;
        ex_toggleLed(EX_LED_GREEN);
    }    // printk("+");

    if (Mode == MLINER_M_STOP_MLINE)
        return 0;
    updateMline();
    trg->done = 1; 
    return 0;
}
static int runTESP_Subscribe_Lthread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runTESP_TimReceiver_Lthread);
    if (result == 0)
        TESP_Subscribe_Marker = 1;
    return 0;
}
uint8_t setMlinerMode(const uint16_t address, exactolink_package_result_t mode)
{
	return 0;
}
uint8_t startMliner(void)
{

    if (Mode != MLINER_M_FIRST_START)
    {
        Mode = MLINER_M_NONE;
        return 0;
    }
    



    if(TESP_Subscribe_Marker == 0)
    {
        lthread_launch(&TESP_Subscribe_Lthread);

        while(!TESP_Subscribe_Marker)
            sleep(1);
        printf("Subscribing is done\n");
    }

    if (TESP_PrintToSD_enabled != 2)
    {
        TESP_PrintToSD_enabled = 1;
        int index = 0;
        while ( TESP_PrintToSD_enabled != 2)
        {
            index++;
            sleep(1); 
            if (index > 20)
            {
                printf("Can't start SD server\n");
                return 1;
            }
        }
    }
    
    printf("Create file\n\n\n");
    ex_setFreqHz(100);
    Mode = MLINER_M_NONE;
    ex_enableGpio(EX_GPIO_SPI_MLINE);
    syncMasterSpiDma();
    return 0;
}
uint8_t stopMliner(void)
{
    Mode = MLINER_M_STOP_ALL;
    return 0;
}
static int runGetValue_Lthread(struct lthread * self)
{
    if (GetValueType == TX_BUFFER)
        *GetValuePtr = TESMAF_Tx_Buffer;
    else if (GetValueType == RX_BUFFER)
        *GetValuePtr = TESMAF_Rx_Buffer;
    else if (GetValueType == DATACHECK_CNTBUFF)
        *GetValuePtr = TESMAF_DataCheck_CntBuff;
    else if (GetValueType == DATACHECK_COUNTER)
        *GetValuePtr = TESMAF_DataCheck_Counter;
    else if (GetValueType == DATACHECK_SCSBUFF)
        *GetValuePtr = TESMAF_DataCheck_ScsBuff;
    else if (GetValueType == DATACHECK_SUCCESS)
        *GetValuePtr = TESMAF_DataCheck_Success;
    else if (GetValueType == TEST_CALLFUNTOOMANYFAILED)
        *GetValuePtr = TESMAF_test_CallFunTooManyFailed;
    else if (GetValueType == TEST_INPUTLST)
        *GetValuePtr = TESMAF_test_InputLst;
    else if (GetValueType == TEST_UPDTELST)
        *GetValuePtr = TESMAF_test_UpdteLst;

    *GetValueRes = 1;
    return 0;
}
void getMlinerVars(mliner_main_mod_vars_t type, uint32_t * ptr, uint8_t * check)
{
    while(!*GetValueRes);
    ptr = GetValuePtr;
    check = GetValueRes;
    *GetValueRes = 0;
    *GetValuePtr = 0;
    GetValueType = type;
    lthread_launch(&GetValue_Lthread);
    while(!*GetValueRes);
}
EMBOX_UNIT_INIT(initMlinerMainMod);
static int initMlinerMainMod(void)
{
    *GetValuePtr = 0;
    *GetValueRes = 1;
    lthread_init(&GetValue_Lthread, runGetValue_Lthread);

    lthread_init(&TESP_Subscribe_Lthread, &runTESP_Subscribe_Lthread);


    TESP_PrintToSD_Thread = thread_create( THREAD_FLAG_DETACHED | THREAD_FLAG_SUSPENDED , runTESP_PrintToSD_Thread, NULL);
    mutex_init(&TESP_PrintToSD_Mutex);
    cond_init(&TESP_PrintToSD_Signal, NULL);
    lthread_init(&TESP_PrintToSD_Remainder_Lthread, runTESP_PrintToSD_Remainder_Lthread);

    lthread_init(&TESMAF_CheckExactoStorage_Lthread, runTESMAF_CheckExactoStorage_Lthread);
    lthread_init(&TESMAF_AfterCheckExStr_Lthread, runTESMAF_AfterCheckExStr_Lthread);

    schedee_priority_set(&TESP_PrintToSD_Thread->schedee, 200);
    schedee_priority_set(&TESP_PrintToSD_Remainder_Lthread.schedee, 220);
    schedee_priority_set(&TESMAF_AfterCheckExStr_Lthread.schedee, 225);
    schedee_priority_set(&TESMAF_CheckExactoStorage_Lthread.schedee, 230);

    TESP_PrintToSD_enabled = 0; 
    
    thread_launch(TESP_PrintToSD_Thread);

	return 0;
}


