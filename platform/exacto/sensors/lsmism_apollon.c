#include "sensors/lsmism.h"

#include "tim/tim.h"
#include <embox/unit.h>
#include <hal/reg.h>

#include "kernel/printk.h"
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"

#include "spi/spi_sns.h"


#define PRINT_TICKER_MAX 9 
#define TRANSMIT_MESSAGE_SIZE EXACTOLINK_MESSAGE_SIZE
#define TMP_BUFFER_DATA_SZ 40
#define RX_DATA_SZ 40

exmliner_dev_t LsmIsmDev;

uint32_t Apollon_lsmism_MlineOverFlow = 0;
uint32_t Apollon_lsmism_MlineReceive = 0;
//uint32_t Apollon_lsmism_MlineTransmit = 0;
volatile uint8_t Apollon_lsmism_MlineRXTx_Readable = 0;
uint32_t Apollon_lsmism_Ticker_Buf = 0;
volatile uint8_t Apollon_lsmism_Ticker_Readable = 0;
volatile uint8_t Apollon_lsmism_Buffer_Readable = 0;
volatile int16_t Apollon_lsmism_Buffer_Data0[3] = {0};
volatile int16_t Apollon_lsmism_Buffer_Data1[3] = {0};
volatile int16_t Apollon_lsmism_Buffer_Data2[3] = {0};
volatile uint8_t Apollon_lsmism_Buffer_dtrd0 = 0;
volatile uint8_t Apollon_lsmism_Buffer_dtrd1 = 0;

static exactolink_package_result_t Mode = EXACTOLINK_SNS_XL_0100_XLGR_0100;
static uint8_t InitFlag = 0;

// static uint8_t Ender[] = {5,5,5,5};
// #define SNS_SERVICE_TESTING
static uint32_t PackRecvCounter = 0;
static uint32_t Counter = 0;

//static unsigned int Delay = 100;

static ex_sns_lth_container_t SnsContainer;

static uint8_t TmpBufferData[TMP_BUFFER_DATA_SZ] = {0};
static uint16_t TmpBufferPtr = 0;


static uint8_t Ticker_Enable = 0;
static uint32_t Ticker_Start = 0;
static uint32_t Ticker_Stop = 0;
static uint32_t Ticker_Res = 0;
static uint32_t Ticker_Cnt = 0;


// static uint16_t OverFlow = 0;

static struct lthread Init_Lthread;

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

static uint8_t sendOptionsRaw(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value, const uint16_t try_count)
{
    uint16_t try_index = 0;
    PackageToSend.data[0] = address;
    PackageToSend.data[1] = value;
    PackageToSend.datalen = 2;
    PackageToSend.type = EX_SPI_DT_TRANSMIT;
    sched_lock();
    enableExactoSensor(sns);
    while(ex_sendSpiSns(&PackageToSend))
    {
        try_index++;
        if (try_index > try_count)
            break;
    }
    disableExactoSensor(sns);
    try_index = 0;
    if (try_count)
    {
        PackageToGett.cmd = address ;
        PackageToGett.datalen = 2;
        enableExactoSensor(sns);
        while(ex_gettSpiSns(&PackageToGett))
        {
            try_index++;
            if (try_index > try_count)
                break;
        }
        disableExactoSensor(sns);
    }
    sched_unlock();
    if (PackageToGett.data[0] != value)
        return 1;
    return 0;
}
static uint8_t getDataFromSns(ex_sns_cmds_t * sns, uint8_t * trg, uint16_t * ptr)
{
	if (sns->cnt_cur < sns->cnt_max)
	{
		sns->cnt_cur++;
		return 0;
	}
	sns->cnt_cur = 0;
	PackageToGett.result = EXACTO_WAITING;
	PackageToGett.cmd = sns->address;//cmd;
	PackageToGett.datalen = sns->datalen+1;
	PackageToGett.type = EX_SPI_DT_TRANSMIT_RECEIVE;
	uint8_t process_completed = 1;
	const uint8_t tmp_length = (sns->datalen - sns->shift);
	enableExactoSensor(sns->sns);
	if (ex_gettSpiSns(&PackageToGett))
		process_completed = 0;
	disableExactoSensor(sns->sns);
	if(isXlGrDataReady(sns->sns, PackageToGett.data[0]) && process_completed)
	{
		exmliner_run(&LsmIsmDev, &PackageToGett.data[sns->shift], tmp_length, (uint8_t)sns->sns);
		
		sns->dtrd = 1;
		*ptr += tmp_length + 2;
		return (tmp_length + 2);
	}
	return 0;
}

uint8_t switchStage(const exactolink_package_result_t type)
{
//    uint8_t value_sns_option_lsm303ah = 0xC5;       //1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
//    uint8_t value_sns_option_ism330dlc_xl = 0x44;   //0100 01 0 0 : 104 Hz 16g 
//    uint8_t value_sns_option_ism330dlc_gr = 0x4c;   //0100 11 0 0 : 104 Hz 2000 dps 

	const uint8_t try_cnt = 5;
	switch (type)
	{
	    case EXACTOLINK_CRC_ERROR:
	    case EXACTOLINK_NO_DATA:
	    return 1;
	    case EXACTOLINK_SNS_XL_0100_XLGR_0100:
	    ex_setFreqHz(100);
	    sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xc5, try_cnt);	//1100 01 0 1 : 100 Hz 16g HF_ODR= 0 BDU=1
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x44, try_cnt);	//0100 01 0 0 : 104 Hz 16g 
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x4c, try_cnt);	//0100 11 0 0 : 104 Hz 2000 dps
	    SnsContainer.sns[0].cnt_max = 0;
	    SnsContainer.sns[1].cnt_max = 0;
	break;
	    case EXACTOLINK_SNS_XL_0200_XLGR_0100:
	    ex_setFreqHz(200);
	    sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xd5, try_cnt);	//1101 01 0 1 : 200 Hz 16g HF_ODR= 0 BDU=1
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x40, try_cnt);	//0100 00 0 0 : 104 Hz 2g 
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x4c, try_cnt);	//0100 11 0 0 : 104 Hz 2000 dps
	    SnsContainer.sns[0].cnt_max = 0;
	    SnsContainer.sns[1].cnt_max = 2;
	break;
	    case EXACTOLINK_SNS_XL_0400_XLGR_0100:
	    ex_setFreqHz(400);
	    sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xe1, try_cnt);	//1110 00 0 1 : 400 Hz 2g HF_ODR= 0 BDU=1
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x40, try_cnt);	//0100 00 0 0 : 104 Hz 2g 
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x40, try_cnt);	//0100 00 0 0 : 104 Hz 245 dps FS on 125 = off
	    SnsContainer.sns[0].cnt_max = 0;
	    SnsContainer.sns[1].cnt_max = 4;
	break;
	    case EXACTOLINK_SNS_XL_0800_XLGR_0100:
	    ex_setFreqHz(800);
	    sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xf5, try_cnt);	//1111 01 0 1 : 800 Hz 16g HF_ODR= 0 BDU=1
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x40, try_cnt);	//0100 00 0 0 : 104 Hz 2g 
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x4c, try_cnt);	//0100 11 0 0 : 104 Hz 2000 dps
	    SnsContainer.sns[0].cnt_max = 0;
	    SnsContainer.sns[1].cnt_max = 8;
	break;
	    case EXACTOLINK_SNS_XL_1600_XLGR_0100:
	    ex_setFreqHz(1600);
	    sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0x57, try_cnt);	//0101 01 1 1 : 800 Hz 16g HF_ODR= 1 BDU=1
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x40, try_cnt);	//0100 00 0 0 : 104 Hz 2g 
	    sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x4c, try_cnt);	//0100 11 0 0 : 104 Hz 2000 dps
	    SnsContainer.sns[0].cnt_max = 0;
	    SnsContainer.sns[1].cnt_max = 16;
		 break;
		case EXACTOLINK_SNS_XL_0200_XLGR_0200:
			ex_setFreqHz(200);
			sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xd1, try_cnt);	//1101 00 0 1 : 200 Hz 2g HF_ODR= 0 BDU=1
			sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x50, try_cnt);	//0101 00 0 0 : 208 Hz 2g 
			sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x50, try_cnt);	//0101 00 0 0 : 208 Hz 245 dps FS on 125 = off
			SnsContainer.sns[0].cnt_max = 0;
			SnsContainer.sns[1].cnt_max = 0;
		break;
		case EXACTOLINK_SNS_XL_0400_XLGR_0400:
			ex_setFreqHz(400);
			sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A,		0xd1, try_cnt);	//1101 00 0 1 : 400 Hz 2g HF_ODR= 0 BDU=1
			sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL,	0x60, try_cnt);	//0110 00 0 0 : 416 Hz 2g 
			sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 	0x60, try_cnt);	//0110 00 0 0 : 416 Hz 245 dps FS on 125 = off
			SnsContainer.sns[0].cnt_max = 0;
			SnsContainer.sns[1].cnt_max = 0;
		break;

    // case EXACTOLINK_LSM303AH_TYPE0:
    //     sendOptionsRaw(LSM303AH, ISM330DLC_CTRL1_XL, 0x44, 5);
    //     for (uint8_t i = 0; i < SnsContainer.sns_count; i++)
    //     {
    //         if (SnsContainer.sns[i].sns == LSM303AH)
    //             SnsContainer.sns[i].isenabled = 1;
    //         else
    //             SnsContainer.sns[i].isenabled = 0;
    //     }
    //     break;
    // case EXACTOLINK_SNS_XLXLGR:
    //     sendOptionsRaw(LSM303AH, LSM303AH_CTRL1_A, 0xC5, 5);
    //     sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL1_XL, 0x44, 5);
    //     sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL2_G, 0x4c, 5);
    //     for (uint8_t i = 0; i < SnsContainer.sns_count; i++)
    //     {
    //         if ((SnsContainer.sns[i].sns == LSM303AH)||(SnsContainer.sns[i].sns == ISM330DLC))
    //             SnsContainer.sns[i].isenabled = 1;
    //         else
    //             SnsContainer.sns[i].isenabled = 0;
    //     }
    //     break;
    default:
        break;
    }
    return 0;
}

static int runSnsContainerLthread(struct lthread * self)
{
	ex_subs_service_t * trg = (ex_subs_service_t*)self;
	if (!Ticker_Enable)
	{
		Ticker_Enable = 1;
		Ticker_Start = dwt_cyccnt_start();
		Ticker_Cnt =0;
		Ticker_Res = 0;
	}
	else
	{

		Ticker_Stop = dwt_cyccnt_stop();
		Ticker_Res += Ticker_Stop - Ticker_Start;
		Ticker_Cnt++;
		Ticker_Start = dwt_cyccnt_start();
		if (Apollon_lsmism_Ticker_Readable == 0)
		{
			Apollon_lsmism_Ticker_Readable = 1;
			Apollon_lsmism_Ticker_Buf = Ticker_Res/Ticker_Cnt;
			Ticker_Cnt = 0;
			Ticker_Res = 0;
		}
	}
	getDataFromSns(&SnsContainer.sns[0], &TmpBufferData[0], & TmpBufferPtr);
	getDataFromSns(&SnsContainer.sns[1], &TmpBufferData[0], & TmpBufferPtr);
	SnsContainer.done = 1;
	Counter++;
	if (SnsContainer.sns[0].dtrd && SnsContainer.sns[1].dtrd)
	{
		SnsContainer.sns[0].dtrd = 0;
		SnsContainer.sns[1].dtrd = 0;
	}
	trg->done = 1;
	return 0;
}
static int run_Init_Lthread(struct lthread * self)
{
	Counter = 0;
	PackRecvCounter = 0;
	Apollon_lsmism_Ticker_Readable = 0;
	Ticker_Enable = 0;
	SnsContainer.sns_count = 2;
	SnsContainer.sns[0].isenabled = 1;
	SnsContainer.sns[0].sns = LSM303AH;
	SnsContainer.sns[0].address = LSM303AH_STATUS_A; 
	SnsContainer.sns[0].datalen = 7;
	SnsContainer.sns[0].pt2buffer = 0;
	SnsContainer.sns[0].shift = 1;
	SnsContainer.sns[0].counter = 0;
	SnsContainer.sns[0].cnt_cur = 0;
	SnsContainer.sns[0].cnt_max = 0;
	SnsContainer.sns[0].dtrd = 0;

	SnsContainer.sns[1].isenabled = 1;
 	SnsContainer.sns[1].sns = ISM330DLC;
	SnsContainer.sns[1].address = ISM330DLC_STATUS_REG; 
	SnsContainer.sns[1].datalen = 16;
	SnsContainer.sns[1].pt2buffer = 6;
	SnsContainer.sns[1].shift = 4;
	SnsContainer.sns[1].counter = 0;
	SnsContainer.sns[1].cnt_cur = 0;
	SnsContainer.sns[1].cnt_max = 0;
	SnsContainer.sns[1].dtrd = 0;

	SnsContainer.done = 0;
    
	switchStage(Mode);
	ex_frcTimReload();
	InitFlag = 1;
    return 0;
}

uint8_t exSnsStart(const uint8_t type)
{
	Mode = (exactolink_package_result_t)(type);

	InitFlag = 0;
	lthread_launch(&Init_Lthread);
	while(InitFlag == 0)
		sleep(1);
	dwt_cyccnt_reset();

	if ( ex_subscribeOnEvent(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, runSnsContainerLthread) != 0)
	        return 1;

	return 0;
}
uint8_t exSnsStop(void)
{
	return 0;
}
EMBOX_UNIT_INIT(initApollon_lsmism);
static int initApollon_lsmism()
{
	exmliner_cr(&LsmIsmDev);
	sendOptionsRaw(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL, 0);
	sendOptionsRaw(ISM330DLC, ISM330DLC_CTRL3_C, 0x4c, 0); // BOOT BDU H_ACTIVE PP_OD SIM IF_INC BLE SW_RESET=0 1 0 0 1 1 0 0

	lthread_init(&Init_Lthread, run_Init_Lthread);
	return 0;
}

