#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <util/err.h>

#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/task.h>
#include <kernel/time/ktime.h>
#include <kernel/printk.h>



#include <sys/wait.h>
#include "tim/tim.h"
#include "ex_utils.h"
#include "gpio/gpio.h"

uint8_t TET_print_Marker = 0;
uint8_t TET_subscribe_Marker = 0;

uint16_t TET_TimEvent_Counter = 0;
uint16_t TET_TimEvent_Buffer = 0;
uint16_t TET_TimEvent_Max = 50;

uint16_t TET_PrintEvent_Max = 0;
uint16_t TET_PrintEvent_Counter = 0;
static struct lthread TET_Subcribe_Lthread;
static struct lthread TET_SafeCopyResult_Lthread;

uint32_t    TET_Ticker_Start, 
            TET_Ticker_Stop, 
            TET_Ticker_Result,
            TET_Ticker_ResultPlus = 0,
            TET_Ticker_Buffer,
            TET_Ticker_BufferPlus = 0;


uint8_t TET_Ticker_Marker = 0;

#define TET_TICKER_ARRAY_SZ 1000
uint32_t TET_Ticker_Array[TET_TICKER_ARRAY_SZ] = {0};
uint16_t TET_Ticker_ArraySz = 0;

uint8_t TET_Gpio_enabled = 0;


static int runTET_SafeCopyResult_Lthread (struct lthread * self)
{
    TET_TimEvent_Buffer = TET_TimEvent_Counter;
    TET_Ticker_Buffer = TET_Ticker_Result;
    TET_Ticker_BufferPlus = TET_Ticker_ResultPlus;
    TET_Ticker_ResultPlus = 0;
    TET_Ticker_ArraySz = TET_TimEvent_Counter;
    // TET_Ticker_Buffer = TET_Ticker_Start;
    
    TET_print_Marker = 1;
//    printk("1\n");
    return 0;
}

static int runTET_TimReceiver_Lthread(struct  lthread * self)
{
    if(TET_Gpio_enabled)
    {
        ex_disableGpio(EX_GPIO_SPI_SYNC);
        TET_Gpio_enabled = 0;
    }
    else
    {
        ex_enableGpio(EX_GPIO_SPI_SYNC);
        TET_Gpio_enabled = 1;
    }
    TET_TimEvent_Counter++;
    if (!TET_Ticker_Marker)
    {
        TET_Ticker_Start = ex_dwt_cyccnt_start();
        TET_Ticker_Marker = 1;
    }
    else
    {
        TET_Ticker_Stop = ex_dwt_cyccnt_stop();
        TET_Ticker_Result = TET_Ticker_Stop - TET_Ticker_Start;
        if (TET_TimEvent_Counter < TET_TICKER_ARRAY_SZ)
            TET_Ticker_Array[TET_TimEvent_Counter] = TET_Ticker_Result;
        TET_Ticker_ResultPlus += TET_Ticker_Result;
        TET_Ticker_Start = ex_dwt_cyccnt_start();
    }
    if (TET_PrintEvent_Counter < TET_PrintEvent_Max)
    {
        TET_PrintEvent_Counter++;
    }
    else
    {
        TET_PrintEvent_Counter = 0;
        //Some happen
        // I generate marker!!!
        if (!TET_print_Marker)
        lthread_launch(&TET_SafeCopyResult_Lthread);
    }
    return 0;
}
static int runTET_Subcribe_Lthread( struct lthread * self)
{
    if (TET_subscribe_Marker)
        return 0;
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTET_TimReceiver_Lthread);
    if (result == 0)
        TET_subscribe_Marker = 1;
    return 0;
}
int main(int argc, char *argv[]) {
    // ex_setOutputGpio(EX_GPIO_SPI_MLINE);
    ex_disableGpio(EX_GPIO_SPI_SYNC);
    TET_print_Marker = 0;
    TET_TimEvent_Counter = 0;
    TET_TimEvent_Buffer = 0;
    TET_TimEvent_Max = 50;
    TET_PrintEvent_Max = 0;
    TET_PrintEvent_Counter = 0;
    TET_Ticker_ResultPlus = 0,
    TET_Ticker_BufferPlus = 0;
    TET_Ticker_Marker = 0;
    TET_Ticker_ArraySz = 0;
    printf("Testing command for TIM\nYou can use following values:\n 10, 50, 100, 200, 400, 800, 1000, 2000\n");
    int value = 0;
    if (argc > 1)
    {
        value = atoi(argv[1]); 
        printf("value: %d",value);
    }
    else
    {
        printf("Specify value!\n");
        return 0;
    }
    switch (value)
    {
    case 10:
        ex_setFreqHz(10);
        break;
    case 50:
        ex_setFreqHz(50);
        break;
    case 100:
        ex_setFreqHz(100);
        break;
    case 200:
        TET_PrintEvent_Max = 19;
        TET_TimEvent_Max = 100;
        ex_setFreqHz(200);
        break;
    case 400:
        TET_PrintEvent_Max = 39;
        TET_TimEvent_Max = 200;
        ex_setFreqHz(400);
        break;
    case 800:
        TET_PrintEvent_Max = 79;
        TET_TimEvent_Max = 400;
        ex_setFreqHz(800);
        break;
    case 1000:
        TET_PrintEvent_Max = 79;
        TET_TimEvent_Max = 400;
        ex_setFreqHz(1000);
        break;
    case 1600:
        TET_PrintEvent_Max = 159;
        TET_TimEvent_Max = 800;
        ex_setFreqHz(1600);
        break;
    case 2000:
        TET_PrintEvent_Max = 159;
        TET_TimEvent_Max = 800;
        ex_setFreqHz(2000);
        break;
    case 3200:
        TET_PrintEvent_Max = 319;
        TET_TimEvent_Max = 1600;
        ex_setFreqHz(3200);
        break;
    case 6400:
        TET_PrintEvent_Max = 639;
        TET_TimEvent_Max = 3200;
        ex_setFreqHz(6400);
        break;
    default:
        printf("This value is not allowed\n");
        return 0;
        break;
    }
    ex_dwt_cyccnt_reset();
    lthread_init(&TET_Subcribe_Lthread, runTET_Subcribe_Lthread);
    lthread_init(&TET_SafeCopyResult_Lthread, runTET_SafeCopyResult_Lthread);
    lthread_launch(&TET_Subcribe_Lthread);

    while(!TET_subscribe_Marker)
    {

    }
    printf("Subscribing is done\n");
    while(TET_TimEvent_Buffer < TET_TimEvent_Max)
    {
        while(!TET_print_Marker){}
        TET_print_Marker = 0;
        //printk("0\n");
        printf("Tim counter %d\nTicker = %d Plus = %d\n", TET_TimEvent_Buffer, TET_Ticker_Buffer, TET_Ticker_BufferPlus);
    }
    printf("\nTicker info:\n[");
    for (uint16_t i = 0; (i < TET_Ticker_ArraySz)&&(i < TET_TICKER_ARRAY_SZ); i ++)
    {
        printf("%d, ", TET_Ticker_Array[i]);
        if (!(i % 10))
            printf("\n");
    }
    printf("]\nDone \n");
    return 0;
}
