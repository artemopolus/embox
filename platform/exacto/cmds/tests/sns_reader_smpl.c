#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_sns_ctrl.h"
#include "spi/spi_sns.h"
#include "tim/tim.h"
#include <embox/unit.h>

#define PRINT_ON
#define PRINT_TICKER_MAX 9 
#define TRANSMIT_MESSAGE_SIZE 64

//===================================
uint8_t SnsStatus = 0x00f;
// 0x01 - lsm303
// 0x02 - mg
// 0x04 - ism330
// 0x08 - bmp280
//===================================

static ex_spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static ex_spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};
typedef struct{
    struct lthread thread;
    uint8_t data[EX_SPI_PACK_SZ];
    uint8_t datalen;
}thread_container_t;

uint8_t Marker = 0;

uint8_t MarkerRxTx = 0;
uint8_t MarkerStage = 0;

uint8_t SRS_MarkerReceive = 0;
uint8_t SRS_MarkerTransmit = 0;


uint8_t EndCicleMarker = 0;

uint8_t SendMarker = 0;


uint16_t SensorTickerCounter = 0;
uint8_t MarkerSubscribe = 0;

exacto_sensors_list_t CurrentTargetSensor = LSM303AH;
uint8_t CurrentTargetSensor_isenabled  = 0;

struct lthread SubscribeThread;
struct lthread ReceiveDataEventThread;
struct lthread TransmitDataEventThread;
struct lthread SendThread;


uint8_t PrintTickerMarker = 0;
uint16_t PrintTickerCounter = 0;


//========================================================================

//start - 5
//acc_lsm303 - 6
//acc ism330 - 6
//gyr ism330 - 6
//magn - 6
//bar - 6
//sum - 35
//result - 64

uint8_t DataToBuffer[TRANSMIT_MESSAGE_SIZE] = {0};


uint8_t BufferToData[TRANSMIT_MESSAGE_SIZE] = {0};

uint8_t Header[] = {7,7,7,7};
uint8_t Ender[] = {5,5,5,5};

//========================================================================

void executeStage();
void uploadRecevedData( const uint8_t pt)
{
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        if (pt+i < TRANSMIT_MESSAGE_SIZE)
            DataToBuffer[pt+i] = ctrl_value;
    }

}
static int runSensorTickerThread(struct lthread * self)
{
    SensorTickerCounter++;
    if (!SendMarker)
        SendMarker = 1;
    
    return 0;
}
static int runReceiveDataEventThread(struct lthread * self)
{
    disableExactoSensor(CurrentTargetSensor);
    CurrentTargetSensor_isenabled = 0;
    ex_gettSpiSns(&PackageToGett);
    uploadRecevedData(0);
    SRS_MarkerReceive = 1;
    ex_runTransmiter();
    
    return 0;
}

static int runTransmitDataEventThread(struct lthread * self)
{
    SRS_MarkerTransmit = 1;
    if (PackageToSend.type == EX_SPI_DT_TRANSMIT)
    {
        CurrentTargetSensor_isenabled = 0;
        disableExactoSensor(CurrentTargetSensor);
        return 0;
    }
    else if (PackageToSend.type == EX_SPI_DT_TRANSMIT_RECEIVE)
    {
        ex_runReceiver();
        return 0;
    }
    return 0;
}
static int runSubscribeThread(struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runSensorTickerThread);
    result |= ex_subscribeOnEvent(&ExSnsServicesInfo, ExSnsServices, THR_SPI_TX, runTransmitDataEventThread);
    result |= ex_subscribeOnEvent(&ExSnsServicesInfo, ExSnsServices, THR_SPI_RX, runReceiveDataEventThread);
    if (result == 0)
        MarkerSubscribe = 1;
    return 0;
}


uint8_t setPackageToGettToNull()
{
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        PackageToGett.data[i] = 0;
    }
    PackageToGett.result = EXACTO_WAITING;    
    return 0;
}
void printReceivedData()
{
    #ifdef PRINT_ON
    // printf("\033[A\33[2K\r");
    printf("Get some data: ");
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        printf("%#04x| ", ctrl_value);
    }
    printf("Counter: %d\n", SensorTickerCounter);
    #endif
}
void sendTxRxPlanned(exacto_sensors_list_t sns, const uint8_t address, uint8_t datalen )
{
    
    
}
void executeStage()
{
    switch (MarkerStage)
    {
    case 0:
        break;
    case 1:
        
        break;
    case 2:
        break;
    case 3:
        break;
    default:
        break;
    }
}
static int runSendThread(struct lthread * self)
{
    SRS_MarkerTransmit = 0;
    SRS_MarkerReceive = 0;
    CurrentTargetSensor_isenabled = 1;
    enableExactoSensor(CurrentTargetSensor);
    ex_sendSpiSns(&PackageToSend);
    return 0;
}
void sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    PackageToSend.data[0] = address & 0x7F;
    PackageToSend.data[1] = value;
    PackageToSend.datalen = 2;
    PackageToSend.type = EX_SPI_DT_TRANSMIT;
    CurrentTargetSensor = sns;
    lthread_launch(&SendThread);
}
void sendAndReceive(exacto_sensors_list_t sns, const uint8_t address)
{
    PackageToSend.data[0] = address | 0x80;
    PackageToSend.datalen = 1;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = EX_SPI_DT_TRANSMIT_RECEIVE;
    CurrentTargetSensor = sns;
    lthread_launch(&SendThread);

}


// EMBOX_UNIT_INIT(initSnsService);
// static int initSnsService(void)
int initSnsService(void)
{
#ifdef PRINT_ON
    printf("Start send data throw spi\n");
#endif
    lthread_init(&SubscribeThread, runSubscribeThread);
    lthread_init(&SendThread, runSendThread);
    lthread_launch(&SubscribeThread);
    sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    resetExactoDataStorage();
    while(!SRS_MarkerTransmit) {}
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR);
        while((!SRS_MarkerTransmit)&&(!SRS_MarkerReceive)) {}
        printReceivedData();
    }
    return 0;
}
int main(int argc, char *argv[]) {
    initSnsService();
    return 0;
}
