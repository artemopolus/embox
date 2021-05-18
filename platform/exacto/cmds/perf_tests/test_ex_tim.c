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
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTET_TimReceiver_Lthread);
    if (result == 0)
        TET_subscribe_Marker = 1;
    return 0;
}
int main(int argc, char *argv[]) {
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
    case 100:
        ex_setFreqHz(100);
        break;
    case 200:
        ex_setFreqHz(200);
        break;
    
    default:
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
