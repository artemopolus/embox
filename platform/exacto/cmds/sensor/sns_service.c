#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_sensors.h"
#include "spi/spi1_generated.h"
#include "tim/tim.h"
#include <embox/unit.h>

// #define PRINT_ON

//===================================
uint8_t SnsStatus = 0x00f;
// 0x01 - lsm303
// 0x02 - mg
// 0x04 - ism330
// 0x08 - bmp280
//===================================

static spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};
typedef struct{
    struct lthread thread;
    uint8_t data[SPI_PACK_SZ];
    uint8_t datalen;
}thread_container_t;

uint8_t Marker = 0;

uint8_t MarkerRxTx = 0;
uint8_t MarkerStage = 0;


uint8_t EndCicleMarker = 0;

uint8_t SendMarker = 0;

thread_container_t CheckDataFormGettThread;
thread_container_t CheckDataFromSendThread;

struct lthread SetPackageToGettToNullThread;
struct lthread UploadDataThread;
struct lthread SetHeaderThread;
struct lthread SetEnderThread;


uint16_t SensorTickerCounter = 0;
uint8_t MarkerSubscribe = 0;
struct lthread SubscribeThread;

uint8_t PrintTickerMarker = 0;
uint16_t PrintTickerCounter = 0;
#define PRINT_TICKER_MAX 9 


//========================================================================

#define TRANSMIT_MESSAGE_SIZE 64

//start - 5
//acc_lsm303 - 6
//acc ism330 - 6
//gyr ism330 - 6
//magn - 6
//bar - 6
//sum - 35
//result - 64

// uint8_t DataToBuffer[TRANSMIT_MESSAGE_SIZE] = {0};
uint8_t DataToBuffer[TRANSMIT_MESSAGE_SIZE] = {0};


uint8_t BufferToData[TRANSMIT_MESSAGE_SIZE] = {0};

uint8_t Header[] = {7,7,7,7};
uint8_t Ender[] = {5,5,5,5};

//========================================================================
static int runSetHeaderThread(struct lthread * self)
{
    clearExactoDataStorage();
   setDataToExactoDataStorage(Header, 4, THR_CTRL_WAIT); 
   MarkerStage = 1;
    return 0;
}
static int runSetEnderThread(struct lthread * self)
{
   setDataToExactoDataStorage(Ender, 4, THR_CTRL_OK); 
   MarkerStage = 0;
    return 0;
}
static int runUploadDataThread(struct lthread * self)
{
    setDataToExactoDataStorage(PackageToGett.data, PackageToGett.datalen, THR_CTRL_WAIT);
    MarkerStage ++;
    return 0;
}
void executeStage();

static int runSensorTickerThread(struct lthread * self)
{
    SensorTickerCounter++;
    if (!SendMarker)
        SendMarker = 1;
    // if (PrintTickerCounter < PRINT_TICKER_MAX)
    // {
    //     PrintTickerCounter++;
    // }
    // else
    // {
    //         PrintTickerCounter = 0;
            executeStage();
    // }
    
    return 0;
}
static int runSubscribeThread(struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runSensorTickerThread);
    if (result == 0)
        MarkerSubscribe = 1;
    return 0;
}


static int setPackageToGettToNull(struct lthread * self)
{
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        PackageToGett.data[i] = 0;
    }
    PackageToGett.result = EXACTO_WAITING;    
    return 0;
}


static int checkDataFromSend( struct lthread * self)
{
    PackageToSend.type = SPI_DT_CHECK;
    if (PackageToSend.result == EXACTO_OK)
    {
        Marker = 1;
    }
    else{
        sendSpi1Half(&PackageToSend);
    }
    
    return 0;
}
static int checkDataFromGet(struct lthread * self)
{
    thread_container_t * _trg;
    _trg = (thread_container_t *)self;
    PackageToGett.type = SPI_DT_RECEIVE;
    PackageToGett.datalen = _trg->datalen;
    if (PackageToGett.result == EXACTO_OK)
    {
        Marker = 1;
        
    }
    else{
        waitSpi1Half(&PackageToGett);
    }
    return 0;
}
void waitUntillEnd(struct lthread * lt)
{
    Marker = 0;
    while (!Marker)
    {
        lthread_launch(lt);
    }
}
void sendAndReceiveShort(exacto_sensors_list_t sns, const uint8_t address, const uint8_t datalen)
{
    enableExactoSensor(sns);
    sendSpi1Half(&PackageToSend);
    CheckDataFormGettThread.datalen = datalen;
    waitUntillEnd(&CheckDataFromSendThread.thread);
    lthread_launch(&SetPackageToGettToNullThread);
    waitUntillEnd(&CheckDataFromSendThread.thread);
    disableExactoSensor(sns);
}
void uploadRecevedData( const uint8_t pt)
{
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        if (pt+i < TRANSMIT_MESSAGE_SIZE)
            DataToBuffer[pt+i] = ctrl_value;
    }

}
void printReceivedData()
{
    #ifdef PRINT_ON
    printf("\033[A\33[2K\rGet some data: ");
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        printf("[%#04x = %d]\t", ctrl_value, ctrl_value);
    }
    printf("Counter: %d\n", SensorTickerCounter);
    #endif
}
void sendTxRxPlanned(exacto_sensors_list_t sns, const uint8_t address, uint8_t datalen )
{
    switch (MarkerRxTx)
    {
    case 0:
        PackageToSend.data[0] = address | 0x80;
        PackageToSend.datalen = 1;
        PackageToSend.result = EXACTO_WAITING;
        PackageToSend.type = SPI_DT_TRANSMIT_RECEIVE;

        enableExactoSensor(sns);
        sendSpi1Half(&PackageToSend);
        CheckDataFormGettThread.datalen = datalen;
        PackageToGett.datalen = datalen;
        MarkerRxTx = 1;
        Marker = 0; 
        break;
    case 1:
        lthread_launch(&CheckDataFromSendThread.thread);
        if (Marker)
            MarkerRxTx = 2;
        break;
    case 2:
        lthread_launch(&SetPackageToGettToNullThread);
        Marker = 0;
        MarkerRxTx = 3;
        break;
    case 3:
        lthread_launch(&CheckDataFormGettThread.thread);
        if (Marker)
            MarkerRxTx = 4;
        break;
    case 4:
        disableExactoSensor(sns);
        MarkerRxTx = 5;
        break;
    default:
        break;
    }
    
}
void executeStage()
{
    // printf("MArkerStage: %d MarkerRxTx: %d\n", MarkerStage, MarkerRxTx);
    switch (MarkerStage)
    {
    case 0:
        lthread_launch(&SetHeaderThread);
        break;
    case 1:
        if (MarkerRxTx == 5)
        {
            MarkerStage = 2;
            MarkerRxTx = 0;
        }
        else
        {
            const uint8_t lsm303ah_statusA = 0x28;
            sendTxRxPlanned(LSM303AH, lsm303ah_statusA, 8);

        }
        break;
    case 2:
        lthread_launch(&UploadDataThread);
        break;
    case 3:
        printReceivedData();
        lthread_launch(&SetEnderThread);
        break;
    default:
        break;
    }
}
void sendAndReceiveCutted(exacto_sensors_list_t sns, const uint8_t address, uint8_t datalen )
{
    PackageToSend.data[0] = address | 0x80;
    PackageToSend.datalen = 1;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT_RECEIVE;

    enableExactoSensor(sns);
    sendSpi1Half(&PackageToSend);
    CheckDataFormGettThread.datalen = datalen;
    PackageToGett.datalen = datalen;
    Marker = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFromSendThread.thread);
    }
    lthread_launch(&SetPackageToGettToNullThread);
    Marker = 0;
    uint16_t counter = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFormGettThread.thread);
        counter++;
        if (counter > 1000)
        {
            #ifdef PRINT_ON
            printf("Failed\n");
            #endif
            return;
        }
    }
    disableExactoSensor(sns);
}
void sendAndReceive(exacto_sensors_list_t sns, const uint8_t address, uint8_t datalen )
{
    PackageToSend.data[0] = address | 0x80;
    PackageToSend.datalen = 1;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT_RECEIVE;

    enableExactoSensor(sns);
    sendSpi1Half(&PackageToSend);
    CheckDataFormGettThread.datalen = datalen;
    Marker = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFromSendThread.thread);
    }
    lthread_launch(&SetPackageToGettToNullThread);
    Marker = 0;
    uint16_t counter = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFormGettThread.thread);
        counter++;
        if (counter > 1000)
        {
            #ifdef PRINT_ON
            printf("Failed\n");
            #endif
            return;
        }
    }
    disableExactoSensor(sns);
    #ifdef PRINT_ON
    printf("Get some data: ");
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        printf("[ %#04x = %d ]\t", ctrl_value, ctrl_value);
    }
    printf("\n");
    #endif
    

}
void sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    PackageToSend.data[0] = address & 0x7F;
    PackageToSend.data[1] = value;
    PackageToSend.datalen = 2;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT;

    enableExactoSensor(sns);
    sendSpi1Half(&PackageToSend);
    Marker = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFromSendThread.thread);
    }
    disableExactoSensor(sns);
 
}
EMBOX_UNIT_INIT(initSnsService);
static int initSnsService(void)
{

    //===========================================================================================


    //===========================================================================================

    lthread_init(&UploadDataThread, runUploadDataThread);
    lthread_init(&SetHeaderThread, runSetHeaderThread);
    lthread_init(&SetEnderThread, runSetEnderThread);
    // const uint8_t adr_mask = 0x7F;

    const uint8_t lsm303ah_3wire_adr = 0x21;
    const uint8_t lsm303ah_3wire_val = 0x07;
    const uint8_t lsm303ah_whoami_xl_adr = 0x0f;
    const uint8_t lsm303ah_whoami_xl_val = 0x43;

    const uint8_t lsm303ah_whoami_mg_adr = 0x4f;
    const uint8_t lsm303ah_whoami_mg_val = 0x40; // or 41

    // //0x12, 0x0C

    const uint8_t ism330dlc_3wire_adr = 0x12;
    const uint8_t ism330dlc_3wire_val = 0x0C;

    const uint8_t ism330dlc_whoami_adr = 0x0F;


    // const uint8_t lsm303ah_dataStart = 0x28;
    const uint8_t ism330dlc_whoami_val = 0x6A;


    const uint8_t lsm303ah_ctrl1 = 0x20;

    const uint8_t lsm303ah_ctrl1_value = 0x39; // 0xb4; //10110100


    // uint8_t data_mas[2] = {0};
    
    lthread_init(&SetPackageToGettToNullThread, setPackageToGettToNull);
    lthread_init(&CheckDataFormGettThread.thread, checkDataFromGet);
    lthread_init(&CheckDataFromSendThread.thread, checkDataFromSend);
#ifdef PRINT_ON

    printf("Start send data throw spi\n");
    // sleep(1);
#endif
    sendOptions(LSM303AH, lsm303ah_3wire_adr, lsm303ah_3wire_val);
#ifdef PRINT_ON
    printf("Options are sended!\n");
    printf("Wait whoami xl value\n");
    // sleep(1);
    //check  xl data
#endif
    for (uint8_t i = 0; i < 9; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_whoami_xl_adr, 3);
    }
    if (PackageToGett.data[1] == lsm303ah_whoami_xl_val)
    {
#ifdef PRINT_ON
        printf("lsm303ah 1 whoami success!\n");
#endif
        SnsStatus |= 0x01;
    }
#ifdef PRINT_ON
    printf("Check whoami lsm303 mg value\n");
#endif
    // sleep(1);
    //check mg data
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_whoami_mg_adr, 3);
    }
    if (PackageToGett.data[1] == lsm303ah_whoami_mg_val)
    {
#ifdef PRINT_ON
        printf("lsm303ah 2 whoami success!\n");
#endif
        SnsStatus |= 0x02;
    }
#ifdef PRINT_ON
    printf("Setup ism330 sensor\n");
#endif
    sendOptions(ISM330DLC, ism330dlc_3wire_adr, ism330dlc_3wire_val);
#ifdef PRINT_ON
    printf("Check whoami ism330 value\n");
#endif
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(ISM330DLC, ism330dlc_whoami_adr, 3);
    }
    if (PackageToGett.data[1] == ism330dlc_whoami_val)
    {
#ifdef PRINT_ON
        printf("ism330 whoami success!\n");
#endif
        SnsStatus |= 0x04;
    }

#ifdef PRINT_ON
    printf("Sensor status: %d\n", SnsStatus);
    printf("Send init params for accelerometer\n");
#endif
    sendOptions(LSM303AH, lsm303ah_ctrl1, lsm303ah_ctrl1_value);
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_ctrl1, 3);
    }
#ifdef PRINT_ON
    printf("Check init params for accelerometer\n");
#endif
    sendOptions(LSM303AH, lsm303ah_ctrl1, lsm303ah_ctrl1_value);
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_3wire_adr, 3);
    }

    lthread_init(&SubscribeThread, runSubscribeThread);
    lthread_launch(&SubscribeThread);

    resetExactoDataStorage();




    return 0;
}
