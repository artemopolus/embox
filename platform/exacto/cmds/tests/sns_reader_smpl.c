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
// #include <stm32f1xx_hal.h>
#include <hal/reg.h>
// #include <asm/arm_m_regs.h>

#define PRINT_ON
#define PRINT_TICKER_MAX 9 
#define TRANSMIT_MESSAGE_SIZE 64

#define DEMCR        0xE000EDFC
#define DEMCR_TRCENA    0x01000000
#define DWT_LAR      0xE0001FB0
#define DWT_LAR_KEY  0xC5ACCE55
#define DWT_CYCCNT   0xE0001004
#define DWT_CTRL     0xE0001000
# define CYCCNTENA   (1 << 0)

static void dwt_cyccnt_reset(void) {
	REG32_ORIN(DEMCR, DEMCR_TRCENA);

	REG32_STORE(DWT_LAR, DWT_LAR_KEY);

	REG32_STORE(DWT_CYCCNT, 0);
}
static inline uint32_t dwt_cyccnt_start(void) {
	REG32_ORIN(DWT_CTRL, CYCCNTENA);

	return REG32_LOAD(DWT_CYCCNT);
}

static inline uint32_t dwt_cyccnt_stop(void) {
	REG32_CLEAR(DWT_CTRL, CYCCNTENA);

	return REG32_LOAD(DWT_CYCCNT);
}
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

uint32_t StartTicker, StopTicker, ResultTicker;
uint8_t StartTickerIsEnabled = 0;

uint8_t Marker = 0;

uint8_t MarkerRxTx = 0;
uint8_t MarkerStage = 0xFF;

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
struct lthread DownloadCmdToSendThread;

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

ex_service_transport_msg_t BufferToData;
// uint8_t BufferToData[TRANSMIT_MESSAGE_SIZE] = {0};

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
    executeStage(); 
    return 0;
}
static int runReceiveDataEventThread(struct lthread * self)
{
    if (StartTickerIsEnabled)
    {
       	StopTicker = dwt_cyccnt_stop();
	    ResultTicker = StopTicker - StartTicker;
        StartTickerIsEnabled = 0;
        
 
    }
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
void setValue( const uint8_t address, const uint8_t value, exacto_sensors_list_t sns )
{
    uint8_t buffer[3] = {0};
    buffer[0] = address;
    buffer[1] = value;
    buffer[2] = (uint8_t) sns;
    ex_uploadDataToServiceMsg(&BufferToData, buffer, 3);
    lthread_launch(&DownloadCmdToSendThread);
}
uint8_t getValue( const uint8_t address, exacto_sensors_list_t sns)
{
    uint8_t buffer[2] = {0};
    buffer[0] = address;
    buffer[1] = (uint8_t)sns;
    ex_uploadDataToServiceMsg(&BufferToData, buffer, 2);
    // if (StartTickerIsEnabled)
	    // StartTicker = dwt_cyccnt_start();

    lthread_launch(&DownloadCmdToSendThread);
    
    if (PackageToGett.result == EXACTO_OK)
        return PackageToGett.data[1];
    else
        return 0;
}
void repeatPrevPackage()
{
    PackageToSend.result = EXACTO_WAITING;
    lthread_launch(&SendThread);
}
void executeStage()
{
    uint8_t value = 0;
    switch (MarkerStage)
    {
    case 0:
        value = getValue(LSM303AH_WHOAMI_XL_ADR, LSM303AH);
        if ( value == LSM303AH_WHOAMI_XL_VAL)
        {
            MarkerStage++;
        }
        break;
    case 1:
        if ( getValue(LSM303AH_WHOAMI_MG_ADR, LSM303AH) == LSM303AH_WHOAMI_MG_VAL)
        {
            MarkerStage++;
        }
        break;
    case 2:
        setValue(LSM303AH_CTRL1_ADR, LSM303AH_CTRL1_VAL, LSM303AH);
        MarkerStage++;
        break;
    case 3:
        if (getValue(LSM303AH_CTRL1_ADR, LSM303AH) == LSM303AH_CTRL1_VAL)
        {
            MarkerStage++;
        }
        break;
    case 4:
	    // StartTicker = dwt_cyccnt_start();
        StartTickerIsEnabled = 1;
	    // dwt_cyccnt_reset();
        // if (StartTickerIsEnabled)
	        // StartTicker = dwt_cyccnt_start();
        getValue(LSM303AH_STATUS_A_ADR, LSM303AH);
        MarkerStage++;
        break;
    case 5:
        // repeatPrevPackage();
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
    if (StartTickerIsEnabled){
	    dwt_cyccnt_reset();
	    StartTicker = dwt_cyccnt_start();
    }
    if (PackageToSend.type == EX_SPI_DT_TRANSMIT)
    {
        ex_sendSpiSns(&PackageToSend);
    }
    else if (PackageToSend.type == EX_SPI_DT_TRANSMIT_RECEIVE)
    {
        ex_gettSpiSns(&PackageToGett);
    }
    disableExactoSensor(CurrentTargetSensor);
    if (StartTickerIsEnabled)
    {
       	StopTicker = dwt_cyccnt_stop();
	    ResultTicker = StopTicker - StartTicker;
        StartTickerIsEnabled = 0;
        
 
    }
    return 0;
}

static int runDownloadCmdToSendThread(struct lthread * self)
{
    uint8_t buffer[3] = {0};
    switch (ex_getLenServiceMsg(&BufferToData))
    {
    case 2:
        if (ex_downloadDataFromServiceMsg(&BufferToData, buffer, 2)) return 0;
        PackageToSend.data[0] = buffer[0] | 0x80;
        PackageToSend.datalen = 1;
        PackageToSend.result = EXACTO_WAITING;
        PackageToSend.type = EX_SPI_DT_TRANSMIT_RECEIVE;
        CurrentTargetSensor = (exacto_sensors_list_t)buffer[1];        break;
    case 3:
        if (ex_downloadDataFromServiceMsg(&BufferToData, buffer, 3)) return 0;
        PackageToSend.data[0] = buffer[0] & 0x7F;
        PackageToSend.data[1] = buffer[1];
        PackageToSend.datalen = 2;
        PackageToSend.result = EXACTO_WAITING;
        PackageToSend.type = EX_SPI_DT_TRANSMIT;
        CurrentTargetSensor = (exacto_sensors_list_t)buffer[2];
        break;
    
    default:
        break;
    }
    // if (StartTickerIsEnabled)
	    // StartTicker = dwt_cyccnt_start();
    lthread_launch(&SendThread);
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
    PackageToGett.cmd = address | 0x80;
    PackageToGett.datalen = 1;
    CurrentTargetSensor = sns;
    lthread_launch(&SendThread);

}


// EMBOX_UNIT_INIT(initSnsService);
// static int initSnsService(void)
int initSnsService(void)
{
    ex_initServiceMsg(&BufferToData);
#ifdef PRINT_ON
    printf("Start send data throw spi\n");
#endif
    lthread_init(&SubscribeThread, runSubscribeThread);
    lthread_init(&SendThread, runSendThread);
    lthread_init(&DownloadCmdToSendThread, runDownloadCmdToSendThread);
    lthread_launch(&SubscribeThread);
    sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    sendOptions(ISM330DLC, 0x12, 0x0c);
    resetExactoDataStorage();
    // while(!SRS_MarkerTransmit) {}
    StartTickerIsEnabled = 1;
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_MG_ADR);
        // while((!SRS_MarkerTransmit)&&(!SRS_MarkerReceive)) {}
        printReceivedData();
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR);
        while((!SRS_MarkerTransmit)&&(!SRS_MarkerReceive)) {}
        printReceivedData();
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(ISM330DLC, 0x0f);
        while((!SRS_MarkerTransmit)&&(!SRS_MarkerReceive)) {}
        printReceivedData();
    }
    return 0;
}
int main(int argc, char *argv[]) {
	// uint32_t start, stop, res;
    initSnsService();


    MarkerStage = 0;
    while (MarkerStage < 5)
    {
    }
    
    printf("Ticker:%d\n", ResultTicker);
    printf("\n\n\n\n");
    while(1)
    {
        printf("\033[A\33[2K\r");
        printf("\033[A\33[2K\r");
        printf("MarkerStage: %d\n", MarkerStage);
        printf("Counter: %d\n",SensorTickerCounter);
        usleep(1000000);

    }

    return 0;
}
