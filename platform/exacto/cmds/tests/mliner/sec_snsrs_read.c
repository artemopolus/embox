#include "mliner/mliner_main.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>

#include "ex_utils.h"
#include "sensors/lsmism.h"
#include "exactolink/exlnk_Data.h"

// Sensor print section

static uint16_t Print_Counter = 0;
static uint16_t Print_MaxCounter = 5;
static uint8_t Print_Mark = 0;
static int16_t Print_Buffer[20] = {0};
static uint32_t Print_ItCounter = 0;

// ===

// Command value


static uint8_t Acc_Mark = 1;
static uint8_t Gyr_Mark = 1;
static uint8_t GXL_Mark = 1;

// ===


#define TIM_1SEC_DIVIDER 200
#define MEASURE_TIME

#ifdef MEASURE_TIME
static exutils_data_t TagTimer;
#endif


static uint16_t TIM_Counter = 0;

static int PointToTim;
static uint8_t EnableUpdate = 0;
static uint8_t NeedToPrint = 0;

static uint8_t Address = 7;

#define SENSOR_DATA_BUFFER_LEN 256
#define GYR_DATA_BUFFER_LEN 512
static uint8_t SensorsDataBuffer[SENSOR_DATA_BUFFER_LEN] = {0};
static uint8_t GyrDataBuffer[GYR_DATA_BUFFER_LEN] = {0};
static exlnk_data_str_t AccData;
static exlnk_data_str_t GyrData;


static uint16_t TIM_Event_Counter = 0;

static void startSensors()
{
	exSnsStart(EXACTOLINK_SNS_XL_0100_XLGR_0100);
}


int printSensorData ()
{
	if (Print_Mark)
	{
		printf("[%d]sensor:"
			"\n"
			, Print_ItCounter++ 
		);
		for (uint8_t i ; i < 3; i++)
		{
			printf("[%8d %8d %8d] ",Print_Buffer[3*i], Print_Buffer[3*i + 1], Print_Buffer[3* i + 2]);
		}
		printf("\n");
		// printf("Cmd mark: %d\n", Command_Mark);
		Print_Mark = 0;
		Acc_Mark = 1;
		Gyr_Mark = 1;
		GXL_Mark = 1;
	}
	return 0;	
}

int onUpdateSensorData(uint8_t * data, uint16_t len, uint8_t id)
{
	if(Acc_Mark && id == LSM303AH )
	{
		for(int i = 0; i < 3; i++)
			exlnk_cv_Uint8_Int16(&data[i*2], (int16_t *)&Print_Buffer[i]);
		Acc_Mark = 0;
	}
	else if ( Gyr_Mark && id == ISM330DLC)
	{
		for(uint8_t i = 0; i < 3; i++)
			exlnk_cv_Uint8_Int16(&data[i*2], (int16_t *)&Print_Buffer[i + 3]);
		Gyr_Mark = 0;
	}	
	else if ( GXL_Mark && id == ISM330DLC_XL)
	{
		for(uint8_t i = 0; i < 3; i++)
			exlnk_cv_Uint8_Int16(&data[i*2], (int16_t *)&Print_Buffer[i + 6]);
		GXL_Mark = 0;
	}
	if(Print_Counter > Print_MaxCounter)
	{
		Print_Counter = 0;
		if(!Print_Mark)
		{
			
			Print_Mark  = 1;
		}
	}
	else
		Print_Counter++;
	return 0;	
}


static int run_Tim_Lthread(struct  lthread * self)
{
	exse_ack(&ExTimServices[PointToTim]);
	if(TIM_Counter < 10000)
		TIM_Counter++;
	else
		TIM_Counter = 0;

	if(NeedToPrint == 0)
	{
		if(! (TIM_Counter % TIM_1SEC_DIVIDER))
		{
			NeedToPrint = 1;
			TIM_Event_Counter ++;
		}
	}
	EnableUpdate = 1;
	return 0;
}

int main(int argc, char *argv[]) 
{
	printf("Basic application\nV 0.1\nAbilities:\n- Send ack\n- Change mode(start/stop sns)\n\n");

	exmliner_init(&LsmIsmDev, onUpdateSensorData);

#ifdef MEASURE_TIME
	ex_dwt_cyccnt_reset();
	exutils_init(&TagTimer);
#endif

	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0, Address);

	exlnk_setData( & AccData, 33, SENSOR_DATA_BUFFER_LEN, SensorsDataBuffer);
	exlnk_setData( & GyrData, 34, GYR_DATA_BUFFER_LEN, GyrDataBuffer);

	startSensors();

	while (1)
	{
		while(!EnableUpdate)
			__asm("nop");
		
		printSensorData();
		
		EnableUpdate = 0;

		usleep(100000);
	}
	
	return 1;
}