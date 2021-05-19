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

#include "commander/exacto_data_storage.h"
#include "commander/exacto_sns_ctrl.h"
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"
#include "spi/spi_sns.h"

#define     TES_TICKER_ARRAY_SZ 1000
uint32_t    TES_Ticker_Array[TES_TICKER_ARRAY_SZ] = {0};
uint16_t    TES_Ticker_ArraySz = 0;
uint32_t    TES_Ticker_Start, 
            TES_Ticker_Stop, 
            TES_Ticker_Result;
uint8_t     TES_Ticker_Marker = 0;

static ex_spi_pack_t TES_PackageToSend = {
    .result = EXACTO_OK,
};
static ex_spi_pack_t TES_PackageToGett = {
    .result = EXACTO_WAITING,
};
exacto_sensors_list_t TES_CurTrgSens = LSM303AH;
uint8_t TES_CurTrgSens_isenabled  = 0;
static struct lthread TES_Send_Lthread;
uint8_t               TES_Send_Marker = 0;

static struct lthread TES_getSensData_Lthread;
static struct lthread TESSB_Subcribe_Lthread;

uint32_t TESSB_Send_Counter = 0;
uint32_t TESSB_Send_Max = 500;
uint32_t TESSB_Recv_Counter = 0;

uint8_t TESSB_PrintWindow_Max = 9;
uint8_t TESSB_PrintWindow_Counter = 0;
uint8_t TESSB_PrintWindow_Marker = 0;
uint8_t TESSB_subscribe_Marker = 0;

void tes_setTickerStart()
{
    TES_Ticker_Start = ex_dwt_cyccnt_start();
}
void tes_setTickerStop()
{
    TES_Ticker_Stop = ex_dwt_cyccnt_stop();
    TES_Ticker_Result = TES_Ticker_Stop - TES_Ticker_Start;
    if (TES_Ticker_ArraySz < TES_TICKER_ARRAY_SZ)
    {
        TES_Ticker_Array[TES_Ticker_ArraySz] = TES_Ticker_Result;
        TES_Ticker_ArraySz++;
    }

}
static int runTESSB_TimReceiver_Lthread(struct  lthread * self)
{
    if (TESSB_PrintWindow_Counter < TESSB_PrintWindow_Max)
    {
        TESSB_PrintWindow_Counter++;
    }
    else
    {
        TESSB_PrintWindow_Counter = 0;
        TESSB_PrintWindow_Marker = 1;
    }
    lthread_launch(&TES_getSensData_Lthread);
    return 0;
}
static int runTESSB_Subcribe_Lthread( struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runTESSB_TimReceiver_Lthread);
    if (result == 0)
        TESSB_subscribe_Marker = 1;
    return 0;
}
void tes_sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    TES_PackageToSend.data[0] = address;
    TES_PackageToSend.data[1] = value;
    TES_PackageToSend.datalen = 2;
    TES_PackageToSend.type = EX_SPI_DT_TRANSMIT;
    TES_CurTrgSens = sns;
    lthread_launch(&TES_Send_Lthread);
}
void tes_sendAndReceive(exacto_sensors_list_t sns, const uint8_t address, const uint8_t datalen)
{
    TES_PackageToSend.result = EXACTO_WAITING;
    TES_PackageToSend.type = EX_SPI_DT_TRANSMIT_RECEIVE;
    TES_PackageToGett.cmd = address ;
    TES_PackageToGett.datalen = datalen;
    TES_CurTrgSens = sns;
    lthread_launch(&TES_Send_Lthread);
}
static int runTES_getSensData_Lthread( struct lthread * self)
{
    // tes_setTickerStart();
    TES_PackageToGett.result = EX_SPI_DT_TRANSMIT_RECEIVE;
    TES_PackageToGett.cmd = LSM303AH_STATUS_A;
    TES_PackageToGett.datalen = 7;
    enableExactoSensor(TES_CurTrgSens);
    ex_gettSpiSns(&TES_PackageToGett);
    disableExactoSensor(TES_CurTrgSens);
    TESSB_Send_Counter++;
    if(isXlGrDataReady(TES_CurTrgSens, TES_PackageToGett.data[0]))
    {
        // tes_setTickerStop();
        // return 1;
        TESSB_Recv_Counter++;
    }
    // tes_setTickerStop();
    TES_Send_Marker = 1;
    return 0;
}
static int runTES_Send_Lthread(struct lthread * self)
{
    TES_Ticker_Start = ex_dwt_cyccnt_start();
    // tes_setTickerStart();
    TES_CurTrgSens_isenabled = 1;
    enableExactoSensor(TES_CurTrgSens);
    if (TES_PackageToSend.type == EX_SPI_DT_TRANSMIT)
    {
        ex_sendSpiSns(&TES_PackageToSend);
    }
    else if (TES_PackageToSend.type == EX_SPI_DT_TRANSMIT_RECEIVE)
    {
        ex_gettSpiSns(&TES_PackageToGett);
    }
    disableExactoSensor(TES_CurTrgSens);
    // tes_setTickerStop();
    return 0;
}
void tessb_printWindow()
{
    printf("DT: ");
    for (uint8_t i = 0; i < TES_PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = TES_PackageToGett.data[i];
        printf(" %d | ", ctrl_value);
    }
    float av = TESSB_Recv_Counter/TESSB_Send_Counter;
    printf(" Tx: %d Rx: %d  Rx/Tx: %.6f \n",TESSB_Send_Counter, TESSB_Recv_Counter, av);
}


int main(int argc, char *argv[]) {
    TES_Ticker_ArraySz = 0;
    TES_Ticker_Marker = 0;
    TES_CurTrgSens_isenabled  = 0;
    TES_Send_Marker = 0;
    TESSB_Send_Counter = 0;
    TESSB_Send_Max = 500;
    TESSB_Recv_Counter = 0;
    TESSB_PrintWindow_Max = 9;
    TESSB_PrintWindow_Counter = 0;
    TESSB_PrintWindow_Marker = 0;
    TESSB_subscribe_Marker = 0;
    int value_input = 0;
    uint8_t value_sns_option = 0xC5;//1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
    if (argc > 1)
    {
        value_input = atoi(argv[1]); 
        printf("value: %d",value_input);
    }
    else
    {
        printf("Specify value!\n");
        return 0;
    }
    switch (value_input)
    {
    case 10:
        TESSB_PrintWindow_Max = 0;
        TESSB_Send_Max = 50;
        ex_setFreqHz(10);
        break;
    case 50:
        TESSB_PrintWindow_Max = 10 - 1;
        TESSB_Send_Max = 100;
        ex_setFreqHz(50);
        break;
    case 100:
        value_sns_option = 0xC5;//1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
        ex_setFreqHz(100);
        break;
    case 200:
        value_sns_option = 0xD5;//1101 01 0 1 : 200 Hz 16g HF_ODR= 0 BDU=1
        TESSB_PrintWindow_Max = 20 - 1;
        TESSB_Send_Max = 1000;
        ex_setFreqHz(200);
        break;
    case 400:
        value_sns_option = 0xE5;//1110 01 0 1 : 400 Hz 16g HF_ODR= 0 BDU=1
        TESSB_PrintWindow_Max = 40 - 1;
        TESSB_Send_Max = 2000;
        ex_setFreqHz(400);
        break;
    case 800:
        value_sns_option = 0xF5;//1111 01 0 1 : 800 Hz 16g HF_ODR= 0 BDU=1
        TESSB_PrintWindow_Max = 80 - 1;
        TESSB_Send_Max = 4000;
        ex_setFreqHz(800);
        break;
    case 1000:
        TESSB_PrintWindow_Max = 80 - 1;
        TESSB_Send_Max = 4000;
        ex_setFreqHz(1000);
        break;
    case 1600:
        value_sns_option = 0x57;//0101 01 1 1 : 1600 Hz 16g HF_ODR= 1 BDU=1
        TESSB_PrintWindow_Max = 160 - 1;
        TESSB_Send_Max = 8000;
        ex_setFreqHz(1600);
        break;
    case 2000:
        TESSB_PrintWindow_Max = 160 - 1;
        TESSB_Send_Max = 8000;
        ex_setFreqHz(2000);
        break;
    default:
        printf("This value is not allowed\n");
        return 0;
        break;
    }


    ex_dwt_cyccnt_reset();
    printf("Start testing sensors\n");
    lthread_init(&TES_Send_Lthread, runTES_Send_Lthread);
    lthread_init(&TES_getSensData_Lthread, runTES_getSensData_Lthread);
    lthread_init(&TESSB_Subcribe_Lthread, runTESSB_Subcribe_Lthread);
    tes_sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    printf("WHOAMI test: ");
    tes_sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR, 2);
    if (TES_PackageToGett.data[0] == LSM303AH_ID_XL)
        printf("Done\n");
    else
        printf("Failed\n");
    printf("Set option test: ");
    tes_sendOptions(LSM303AH, LSM303AH_CTRL1_A, value_sns_option); 
    tes_sendAndReceive(LSM303AH, LSM303AH_CTRL1_A, 2);
    if (TES_PackageToGett.data[0] == value_sns_option)
        printf("Done\n");
    else
        printf("Failed\n");

    lthread_launch(&TESSB_Subcribe_Lthread);

    while(!TESSB_subscribe_Marker){}

    while(TESSB_Send_Counter < TESSB_Send_Max)
    {
        while(!TESSB_PrintWindow_Marker){}
        TESSB_PrintWindow_Marker = 0;
        tessb_printWindow();

    }

    printf("\nTicker info:\n[");
    for (uint16_t i = 0; i < TES_Ticker_ArraySz; i++)
    {
        printf("%d, ", TES_Ticker_Array[i]);
        if (!(i % 10))
            printf("\n");
 
    }
    printf("]\n");
    printf("Selftest done\n");
    return 0;
}
