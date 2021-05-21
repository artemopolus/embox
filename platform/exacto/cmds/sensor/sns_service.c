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
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"
#include "sns_service.h"

// #define PRINT_ON
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
static ex_spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static ex_spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};
#define EX_SNS_CMDS_COUNT 6
typedef struct{
    exacto_sensors_list_t sns;
    uint8_t isenabled;
    uint8_t address;
    uint16_t datalen;
    uint16_t pt2buffer;
    uint8_t shift;
    exacto_dtstr_types_t sns_type;
}ex_sns_cmds_t;
typedef struct{
    struct lthread thread;
    uint8_t sns_count;
    ex_sns_cmds_t sns[EX_SNS_CMDS_COUNT];
    uint8_t sns_current;
}ex_sns_lth_container_t;

uint32_t StartTicker, StopTicker, ResultTicker;
uint8_t StartTickerIsEnabled = 0;


uint8_t MarkerStage = 0xFF;


uint64_t SensorTickerCounter = 0;

uint16_t ExecuteSendCounter = 0;
uint16_t UploadSnsDataCounter = 0;

uint8_t MarkerSubscribe = 0;

exacto_sensors_list_t CurrentTargetSensor = LSM303AH;
uint8_t CurrentTargetSensor_isenabled  = 0;

struct lthread SubscribeThread;
struct lthread SendThread;
ex_sns_lth_container_t SendAndUploadThread;
struct lthread DownloadCmdToSendThread;

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
void sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    PackageToSend.data[0] = address;
    PackageToSend.data[1] = value;
    PackageToSend.datalen = 2;
    PackageToSend.type = EX_SPI_DT_TRANSMIT;
    CurrentTargetSensor = sns;
    lthread_launch(&SendThread);
}
void sendAndReceive(exacto_sensors_list_t sns, const uint8_t address, const uint8_t datalen)
{
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = EX_SPI_DT_TRANSMIT_RECEIVE;
    PackageToGett.cmd = address ;
    PackageToGett.datalen = datalen;
    CurrentTargetSensor = sns;
    lthread_launch(&SendThread);
}
void executeStage();
void uploadRecevedData( const uint8_t pt, const uint16_t start, const uint16_t datalen)
{
    for (uint8_t i = 0; i < datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[start + i];
        if (pt+i < TRANSMIT_MESSAGE_SIZE)
            DataToBuffer[pt+i] = ctrl_value;
    }

}
static int runSensorTickerThread(struct lthread * self)
{
    SensorTickerCounter++;
    
    executeStage(); 
    return 0;
}

static int runSubscribeThread(struct lthread * self)
{
    uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, THR_TIM, runSensorTickerThread);
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

void printDataValues(uint8_t * data, const uint16_t datalen)
{
    #ifdef PRINT_ON

    for (uint8_t i = 0; i < datalen; i++)
    {
        int16_t dst = 0;
        convertUint8ToUint16(&data[i*2], &dst);
        printf("%d\t", dst);
    }
    #endif
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
void printWindow()
{
    #ifdef PRINT_ON
    printf("\033[A\33[2K\r"); //clear line
    printf("\033[A\33[2K\r"); //clear line
    printf("\033[A\33[2K\r"); //clear line
    printf("\033[A\33[2K\r"); //clear line
    printf("\033[A\33[2K\r"); //clear line
    printf("Basic: %d\n", SensorTickerCounter);
    printf("Send: %d\n", ExecuteSendCounter);
    printf("Upload: %d\n", UploadSnsDataCounter);

    printf("lsm303: ");
    printDataValues(&DataToBuffer[SendAndUploadThread.sns[0].pt2buffer], 3);
    printf("\n");
    printf("ism330: ");
    printDataValues(&DataToBuffer[SendAndUploadThread.sns[1].pt2buffer], 6);
    printf("\n");
    #endif
}

void repeatPrevPackage()
{
    PackageToSend.result = EXACTO_WAITING;
    lthread_launch(&SendThread);
}
void executeStage()
{
    // uint8_t value = 0;
    switch (MarkerStage)
    {
    case 0:
#ifdef PRINT_ON
        printf("Start receiving data from sensors\n");
        printf("\n\n\n\n");
#endif
        MarkerStage++; 
        break;
    case 1:
        ExecuteSendCounter ++;
        lthread_launch(&SendAndUploadThread.thread);
        if (PrintTickerCounter < PRINT_TICKER_MAX)
        {
            PrintTickerCounter++;
        }
        else
        {
            PrintTickerCounter = 0;
            printWindow();
        }
        break;
    default:
        break;
    }
}
static int runSendAndUploadThread(struct lthread * self)
{
    ex_sns_lth_container_t * trg = (ex_sns_lth_container_t*)self;
    uint16_t count = trg->sns_count;
    uint16_t enabled = 0;
    // uint8_t counter_tmp;
    // counter_tmp = (uint8_t)(SensorTickerCounter );
    // Header[0] = counter_tmp; 
    // counter_tmp = (uint8_t)(SensorTickerCounter >> 8); 
    // Header[1] = counter_tmp; 
    // counter_tmp = (uint8_t)(SensorTickerCounter >> 16); 
    // Header[2] = counter_tmp; 
    // counter_tmp = (uint8_t)(SensorTickerCounter >> 24); 
    // Header[3] = counter_tmp; 
    // setDataToExactoDataStorage(Header, 4, THR_CTRL_INIT); 

    for (uint16_t i = 0; i < count; i++)
    {
        if(trg->sns[i].isenabled){
            uint8_t cmd = trg->sns[i].address;
            exacto_sensors_list_t sns = trg->sns[i].sns;
            uint16_t datalen = trg->sns[i].datalen;
            uint16_t pt = trg->sns[i].pt2buffer;
            uint8_t shift = trg->sns[i].shift;
            exacto_dtstr_types_t sns_type = trg->sns[i].sns_type;
            PackageToGett.result = EX_SPI_DT_TRANSMIT_RECEIVE;
            PackageToGett.cmd = cmd;
            PackageToGett.datalen = datalen;
            enableExactoSensor(sns);
            ex_gettSpiSns(&PackageToGett);
            disableExactoSensor(sns);
            if(isXlGrDataReady(sns, PackageToGett.data[0]))
            {
                uploadRecevedData(pt, shift, datalen);
                // setDataToExactoDataStorage(&PackageToGett.data[shift], (datalen-shift), THR_CTRL_WAIT);

                ex_setData_ExactoDtStr( &PackageToGett.data[shift],(datalen - shift), SensorTickerCounter, sns_type );
                enabled++;
            }
        }
    }
    // setDataToExactoDataStorage(Ender, 4, THR_CTRL_OK); 

    if (enabled == count)
    {
        UploadSnsDataCounter++;

    }
    return 0;
}
static int runSendThread(struct lthread * self)
{
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
    uint8_t buffer[4] = {0};
    if (ex_downloadDataFromServiceMsg(&BufferToData, buffer, 3))
        return 0;
    switch ((ex_spi_data_type_t)buffer[3])
    {
    case EX_SPI_DT_TRANSMIT_RECEIVE:
        sendAndReceive((exacto_sensors_list_t)buffer[0], buffer[1], buffer[2]);
        break;
    case EX_SPI_DT_TRANSMIT:
        sendOptions((exacto_sensors_list_t)buffer[0], buffer[1], buffer[2]);    
        break;
    default:
        break;
    }
    return 0;
}



EMBOX_UNIT_INIT(initSnsService);
static int initSnsService(void)
// int initSnsService(void)
{
    ex_initServiceMsg(&BufferToData);
#ifdef PRINT_ON
    printf("Start send data throw spi\n");
#endif
    lthread_init(&SubscribeThread, runSubscribeThread);
    lthread_init(&SendThread, runSendThread);
    lthread_init(&SendAndUploadThread.thread, runSendAndUploadThread);
    lthread_init(&DownloadCmdToSendThread, runDownloadCmdToSendThread);
    lthread_launch(&SubscribeThread);
    sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    sendOptions(ISM330DLC, ISM330DLC_CTRL3_C, 0x0c);
    resetExactoDataStorage();
    SendAndUploadThread.sns_count = 2;
    SendAndUploadThread.sns[0].isenabled = 1;
    SendAndUploadThread.sns[0].sns = LSM303AH;
    SendAndUploadThread.sns[0].address = LSM303AH_STATUS_A; 
    SendAndUploadThread.sns[0].datalen = 7;
    SendAndUploadThread.sns[0].pt2buffer = 0;
    SendAndUploadThread.sns[0].shift = 1;
    SendAndUploadThread.sns[0].sns_type = EX_XL_LSM303AH;

    SendAndUploadThread.sns[1].isenabled = 1;
    SendAndUploadThread.sns[1].sns = ISM330DLC;
    SendAndUploadThread.sns[1].address = ISM330DLC_STATUS_REG; 
    SendAndUploadThread.sns[1].datalen = 16;
    SendAndUploadThread.sns[1].pt2buffer = 6;
    SendAndUploadThread.sns[1].shift = 4;
    SendAndUploadThread.sns[1].sns_type = EX_XL_ISM330DLC;
#ifdef PRINT_ON
    printf("lsm303ah XL\n");
#endif
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR, 2);
        printReceivedData();
    }
#ifdef PRINT_ON
    printf("whoami:");
    if (PackageToGett.data[0]== LSM303AH_ID_XL)
        printf("Done\n");
    else
        printf("Failed\n");
    printf("ism330\n");
#endif
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(ISM330DLC, ISM330DLC_WHOAMI_ADR, 2);
        printReceivedData();
    }

#ifdef PRINT_ON
    printf("whoami:");
    if (PackageToGett.data[0]== ISM330DLC_ID)
        printf("Done\n");
    else
        printf("Failed\n");
    printf("lsm303ah MG\n");
#endif
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_MG_ADR, 2);
        printReceivedData();
    }

    sendOptions(ISM330DLC, ISM330DLC_CTRL1_XL, 0x44); //0100 01 0 0
    sendOptions(ISM330DLC, ISM330DLC_CTRL2_G, 0x48); //0100 01 0 0

#ifdef PRINT_ON
    printf("whoami:");
    if (PackageToGett.data[0]== LSM303AH_ID_MG)
        printf("Done\n");
    else
        printf("Failed\n");
    printf("ism330 data read\n");
#endif
    StartTickerIsEnabled = 1;
    for (uint8_t i = 0; i < 6; i++)
    {
        sendAndReceive(ISM330DLC, ISM330DLC_STATUS_REG, 16);
        printReceivedData();
#ifdef PRINT_ON
        printf("\n");
#endif
        printDataValues(&PackageToGett.data[SendAndUploadThread.sns[1].shift], 6);
#ifdef PRINT_ON
        printf("\n");
#endif
    }

#ifdef PRINT_ON
    printf("Ticker counter result: %d\n", ResultTicker);
#endif
    

    sendOptions(LSM303AH, LSM303AH_CTRL1_A, 0xC5);

#ifdef PRINT_ON
    printf("lsm303 data read\n");
#endif
    for (uint8_t i = 0; i < 6; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_STATUS_A, 7);
        printReceivedData();
#ifdef PRINT_ON
        printf("\n");
#endif
        printDataValues(&PackageToGett.data[SendAndUploadThread.sns[0].shift], 3);
#ifdef PRINT_ON
        printf("\n");
#endif
    }

    MarkerStage = 0;

    return 0;
}
uint8_t ex_setFreqHz_SnsService(const uint16_t freq, const exacto_output_state_t state)
{
    ex_stopTimerTIM();
    uint8_t value_sns_option = 0xC5;
    switch (freq)
    {
    case 10:
        ex_setFreqHz(10);
        break;
    case 50:
        ex_setFreqHz(50);
        break;
    case 100:
        value_sns_option = 0xC5;//1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
        ex_setFreqHz(100);
        break;
    case 200:
        value_sns_option = 0xD5;//1101 01 0 1 : 200 Hz 16g HF_ODR= 0 BDU=1
        ex_setFreqHz(200);
        break;
    case 400:
        value_sns_option = 0xE5;//1110 01 0 1 : 400 Hz 16g HF_ODR= 0 BDU=1
        ex_setFreqHz(400);
        break;
    case 800:
        value_sns_option = 0xF5;//1111 01 0 1 : 800 Hz 16g HF_ODR= 0 BDU=1
        ex_setFreqHz(800);
        break;
    case 1600:
        value_sns_option = 0x57;//0101 01 1 1 : 1600 Hz 16g HF_ODR= 1 BDU=1
        ex_setFreqHz(1600);
        break;
    default:
        return 1;
        break;
    }
    sendOptions(LSM303AH, LSM303AH_CTRL1_A, value_sns_option);
    ex_startTimerTIM();
    return 0;
}
// int main(int argc, char *argv[]) {
// 	// uint32_t start, stop, res;
//     initSnsService();


//     MarkerStage = 0;
//     // while (MarkerStage < 5)
//     // {
//     // }
    
//     // printf("Ticker:%d\n", ResultTicker);
//     // printf("\n\n\n\n");
//     while(1)
//     {
//     //     printf("\033[A\33[2K\r");
//     //     printf("\033[A\33[2K\r");
//     //     printf("MarkerStage: %d\n", MarkerStage);
//     //     printf("Counter: %d\n",SensorTickerCounter);
//     //     usleep(1000000);

//     }

//     return 0;
// }
