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

#include "kernel/printk.h"
// #include <asm/arm_m_regs.h>
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"
#include "sensor/sns_service.h"
// #define PRINT_ON
#define PRINT_TICKER_MAX 9 
#define TRANSMIT_MESSAGE_SIZE EXACTOLINK_MESSAGE_SIZE

// #define SNS_SERVICE_TESTING

#ifdef SNS_SERVICE_TESTING

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
#endif
static ex_spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static ex_spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};
#ifdef SNS_SERVICE_TESTING
uint32_t StartTicker, StopTicker, ResultTicker;
uint8_t StartTickerIsEnabled = 0;
#endif

uint8_t MarkerStage = 0xFF;


static uint64_t SensorTickerCounter = 0;

static uint16_t ExecuteSendCounter = 0;
static uint16_t UploadSnsDataCounter = 0;

uint8_t MarkerSubscribe = 0;

static exacto_sensors_list_t CurrentTargetSensor = LSM303AH;
static uint8_t CurrentTargetSensor_isenabled  = 0;

// static struct lthread SubscribeThread;
static struct lthread SendThread;
static ex_sns_lth_container_t SendAndUploadThread;
static struct lthread DownloadCmdToSendThread;
#ifdef PRINT_ON
uint16_t PrintTickerCounter = 0;
#endif


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

static uint8_t Ender[] = {5,5,5,5};

static uint8_t SNSSRV_SensorCheck_Max = 0;
static uint8_t SNSSRV_SensorCheck_Counter = 0;
static uint8_t SNSSRV_UploadData_Max = 0;
static uint8_t SNSSRV_UploadData_Counter = 0;
static uint32_t SNSSRV_PackRecv_Counter = 0;
//========================================================================
void executeStage();
void sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value);
//========================================================================

uint8_t ex_switchStage_SnsService(exactolink_package_result_t type)
{
    exacto_tim_states_t state = ex_getFreqHz_TIM();
    uint8_t value_sns_option_lsm303ah = 0xC5;       //1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
    uint8_t value_sns_option_ism330dlc_xl = 0x44;   //0100 01 0 0 : 104 Hz 16g 
    uint8_t value_sns_option_ism330dlc_gr = 0x4c;   //0100 11 0 0 : 104 Hz 2000 dps 
    switch (state)
    {
    case EXACTO_TIM_10:
    case EXACTO_TIM_50:
    case EXACTO_TIM_100:
        SNSSRV_UploadData_Max = 0;
        SNSSRV_SensorCheck_Max = 0;
        break;
    case EXACTO_TIM_200:
        SNSSRV_UploadData_Max = 2;
        SNSSRV_SensorCheck_Max = 0;
        value_sns_option_lsm303ah = 0xC5;//1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
        value_sns_option_ism330dlc_xl = 0x50;
        value_sns_option_ism330dlc_gr = 0x5c; //0101 11 0 0
        break;
    case EXACTO_TIM_400:
        SNSSRV_UploadData_Max = 4;
        SNSSRV_SensorCheck_Max = 0;
        value_sns_option_lsm303ah = 0xE5;//1110 01 0 1 : 400 Hz 16g HF_ODR= 0 BDU=1
        value_sns_option_ism330dlc_xl = 0x60;
        value_sns_option_ism330dlc_gr = 0x6c; //0110 11 00
        break;
    default:
        break;
    }
    switch (type)
    {
    case EXACTOLINK_CRC_ERROR:
    case EXACTOLINK_NO_DATA:
        return 1;
        break;
    case EXACTOLINK_LSM303AH_TYPE0:
        sendOptions(LSM303AH, LSM303AH_CTRL1_A, value_sns_option_lsm303ah);
        for (uint8_t i = 0; i < SendAndUploadThread.sns_count; i++)
        {
            if (SendAndUploadThread.sns[i].sns == LSM303AH)
            {
                SendAndUploadThread.sns[i].isenabled = 1;
            }
            else
            {
                SendAndUploadThread.sns[i].isenabled = 0;
            }
        }
        break;
    case EXACTOLINK_SNS_XLXLGR:
        sendOptions(LSM303AH, LSM303AH_CTRL1_A, value_sns_option_lsm303ah);
        sendOptions(ISM330DLC, ISM330DLC_CTRL1_XL, value_sns_option_ism330dlc_xl);
        sendOptions(ISM330DLC, ISM330DLC_CTRL2_G, value_sns_option_ism330dlc_gr);
        for (uint8_t i = 0; i < SendAndUploadThread.sns_count; i++)
        {
            if ((SendAndUploadThread.sns[i].sns == LSM303AH)||(SendAndUploadThread.sns[i].sns == ISM330DLC))
            {
                SendAndUploadThread.sns[i].isenabled = 1;
            }
            else
            {
                SendAndUploadThread.sns[i].isenabled = 0;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

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
    
    executeStage(); 
    return 0;
}

// static int runSubscribeThread(struct lthread * self)
// {
//     uint8_t result = ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runSensorTickerThread);
//     if (result == 0)
//         MarkerSubscribe = 1;
//     return 0;
// }


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
#ifdef PRINT_ON
void printWindow()
{
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
}
#endif

// void repeatPrevPackage()
// {
//     PackageToSend.result = EXACTO_WAITING;
//     lthread_launch(&SendThread);
// }
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
#ifdef PRINT_ON
        if (PrintTickerCounter < PRINT_TICKER_MAX)
        {
            PrintTickerCounter++;
        }
        else
        {
            PrintTickerCounter = 0;
            printWindow();
        }
#endif
        break;
    default:
        break;
    }
}
    uint8_t tmp_buffer_data[40] = {0};
static int runSendAndUploadThread(struct lthread * self)
{
    // printk("@");
    ex_sns_lth_container_t * trg = (ex_sns_lth_container_t*)self;
    uint16_t count = trg->sns_count;
    uint16_t enabled = 0;
    uint8_t tmp_buffer_index = 1;
    if (SNSSRV_UploadData_Counter == 0)
    {
        setDataToExactoDataStorage(Ender, 0, EX_THR_CTRL_INIT); 
    }
    if (SNSSRV_SensorCheck_Counter < SNSSRV_SensorCheck_Max)
    {
        SNSSRV_SensorCheck_Counter ++;
    }
    else
    {
        tmp_buffer_data[0] = 0;
        for (uint16_t i = 0; i < count; i++)
        {
            if(trg->sns[i].isenabled){
                uint8_t cmd = trg->sns[i].address;
                exacto_sensors_list_t sns = trg->sns[i].sns;
                uint16_t datalen = trg->sns[i].datalen;
                uint16_t pt = trg->sns[i].pt2buffer;
                uint8_t shift = trg->sns[i].shift;
                PackageToGett.result = EX_SPI_DT_TRANSMIT_RECEIVE;
                PackageToGett.cmd = cmd;
                PackageToGett.datalen = datalen;
                enableExactoSensor(sns);
                uint16_t try_cnt = 0;
                while (ex_gettSpiSns(&PackageToGett))
                {
                    if (try_cnt > 3)
                        break;
                    try_cnt++;
                }
                disableExactoSensor(sns);
                uint8_t tmp_length = (datalen - shift);
                if(isXlGrDataReady(sns, PackageToGett.data[0]))
                {
                    uploadRecevedData(pt, shift, datalen);
                    // setDataToExactoDataStorage(&PackageToGett.data[shift], (datalen-shift), EX_THR_CTRL_WAIT);
                    tmp_buffer_data[0] |= (uint8_t)sns;
                    for (uint8_t i = 0; i < tmp_length; i++)
                    {
                        tmp_buffer_data[tmp_buffer_index + i] = PackageToGett.data[i + shift];
                    }   
                    enabled++;
                }
                else
                {
                    for (uint8_t i = 0; i < tmp_length; i++)
                    {
                        tmp_buffer_data[tmp_buffer_index + i] = 0;
                    }   

                }
                tmp_buffer_index += tmp_length;
            }
        }
        SensorTickerCounter++;
        if (enabled > 0)
        {
            SNSSRV_PackRecv_Counter += enabled;
        }
        SNSSRV_SensorCheck_Counter = 0;
        setDataToExactoDataStorage(tmp_buffer_data, tmp_buffer_index, EX_THR_CTRL_WAIT);
    }

    if (SNSSRV_UploadData_Counter < SNSSRV_UploadData_Max)
    {
        SNSSRV_UploadData_Counter++;
    }
    else
    {
        setDataToExactoDataStorage(Ender, 0, EX_THR_CTRL_OK);
        SNSSRV_UploadData_Counter = 0;
    }

    if (enabled == count)
    {
        UploadSnsDataCounter++;

    }
    // printk("!");
    return 0;
}
static int runSendThread(struct lthread * self)
{
    CurrentTargetSensor_isenabled = 1;
    enableExactoSensor(CurrentTargetSensor);
#ifdef SNS_SERVICE_TESTING
    if (StartTickerIsEnabled){
	    dwt_cyccnt_reset();
	    StartTicker = dwt_cyccnt_start();
    }
#endif
    if (PackageToSend.type == EX_SPI_DT_TRANSMIT)
    {
        ex_sendSpiSns(&PackageToSend);
    }
    else if (PackageToSend.type == EX_SPI_DT_TRANSMIT_RECEIVE)
    {
        ex_gettSpiSns(&PackageToGett);
    }
    disableExactoSensor(CurrentTargetSensor);
#ifdef SNS_SERVICE_TESTING
    if (StartTickerIsEnabled)
    {
       	StopTicker = dwt_cyccnt_stop();
	    ResultTicker = StopTicker - StartTicker;
        StartTickerIsEnabled = 0;
        
 
    }
#endif
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
    // lthread_init(&SubscribeThread, runSubscribeThread);
    lthread_init(&SendThread, runSendThread);
    lthread_init(&SendAndUploadThread.thread, runSendAndUploadThread);
    lthread_init(&DownloadCmdToSendThread, runDownloadCmdToSendThread);
    sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    sendOptions(ISM330DLC, ISM330DLC_CTRL3_C, 0x4c); // 0 1 0 0 1 1 0 0
    resetExactoDataStorage();
    SendAndUploadThread.sns_count = 2;
    SendAndUploadThread.sns[0].isenabled = 1;
    SendAndUploadThread.sns[0].sns = LSM303AH;
    SendAndUploadThread.sns[0].address = LSM303AH_STATUS_A; 
    SendAndUploadThread.sns[0].datalen = 7;
    SendAndUploadThread.sns[0].pt2buffer = 0;
    SendAndUploadThread.sns[0].shift = 1;
    SendAndUploadThread.sns[1].isenabled = 1;
    SendAndUploadThread.sns[1].sns = ISM330DLC;
    SendAndUploadThread.sns[1].address = ISM330DLC_STATUS_REG; 
    SendAndUploadThread.sns[1].datalen = 16;
    SendAndUploadThread.sns[1].pt2buffer = 6;
    SendAndUploadThread.sns[1].shift = 4;
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR, 2);
        // printReceivedData();
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(ISM330DLC, ISM330DLC_WHOAMI_ADR, 2);
        printReceivedData();
    }


    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_WHOAMI_MG_ADR, 2);
        printReceivedData();
    }

    sendOptions(ISM330DLC, ISM330DLC_CTRL1_XL, 0x44); //0100 01 0 0
    sendOptions(ISM330DLC, ISM330DLC_CTRL2_G, 0x48); //0100 01 0 0


#ifdef SNS_SERVICE_TESTING
    StartTickerIsEnabled = 1;
#endif
    for (uint8_t i = 0; i < 3; i++)
    {
        sendAndReceive(ISM330DLC, ISM330DLC_STATUS_REG, 16);
        printReceivedData();
        printDataValues(&PackageToGett.data[SendAndUploadThread.sns[1].shift], 6);
    }

    

    sendOptions(LSM303AH, LSM303AH_CTRL1_A, 0xC5);

    for (uint8_t i = 0; i < 6; i++)
    {
        sendAndReceive(LSM303AH, LSM303AH_STATUS_A, 7);
        printReceivedData();
        printDataValues(&PackageToGett.data[SendAndUploadThread.sns[0].shift], 3);
    }

    MarkerStage = 0;
    // lthread_launch(&SubscribeThread);
    if ( ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runSensorTickerThread) != 0)
        return 1;

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
