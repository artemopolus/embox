#include "mliner/mliner_main.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>

#include "ex_utils.h"
#include "sensors/lsmism.h"
#include "exactolink/exlnk_Data.h"

// Sensor print section

static uint16_t Print_Counter = 0;
static uint16_t Print_MaxCounter = 25;
static int16_t Print_Buffer[20] = {0};
static uint32_t Print_ItCounter = 0;

static uint8_t Print_Mark = 0;
static uint8_t Acc_Mark = 1;
static uint8_t Gyr_Mark = 1;
static uint8_t GXL_Mark = 1;


// ===

// Command value

static uint8_t Command_Mark = EXACTOLINK_NO_DATA;
static uint8_t ModeChange_Mark = 0;
static uint32_t OverLoad_Mark = 0;
static uint32_t DataInputALLCount = 0;

// ===
struct lthread UploadDataThread;


#define TIM_1SEC_DIVIDER 200
#define MEASURE_TIME

#ifdef MEASURE_TIME
static exutils_data_t TagTimer;
static uint32_t 	
						UpdateMlineDuration = 0,
						UpdateMlineDurationAVR = 0,
						TransmitMlineDuration = 0,
						TransmitMlineDurationAVR = 0
						;
#endif


static uint16_t TIM_Counter = 0;
static uint16_t SendCounter = 0;

static int PointToTim;
static uint8_t EnableUpdate = 0;
static uint8_t NeedToPrint = 0;

static uint8_t Address = 7;

#define ACCA_DATA_BUFFER_LEN 256
#define GYR_DATA_BUFFER_LEN 256
#define BCCB_DATA_BUFFER_LEN 256
static uint8_t AccADataBuffer[ACCA_DATA_BUFFER_LEN] = {0};
static uint8_t GyrDataBuffer[GYR_DATA_BUFFER_LEN] = {0};
static uint8_t BccBDataBuffer[BCCB_DATA_BUFFER_LEN] = {0};
static exlnk_data_str_t AccData;
static exlnk_data_str_t GyrData;
static exlnk_data_str_t BccBData;

static uint16_t AccADataUploadLen = 0;
static uint16_t GyrDataUploadLen = 0;
static uint16_t BccBDataUploadLen = 0;
static uint16_t SensorDataUploadCnt = 0;

static uint16_t TIM_Event_Counter = 0;

int ReportStatus()
{
	if (NeedToPrint == 0)
		return 1;
	printf("Tim Events: %d\n", TIM_Event_Counter);
	printf("ACC upload[%d]\n", AccADataUploadLen);
	printf("GYR upload[%d]\n", GyrDataUploadLen);
	printf("SNS Events: %d\n", SensorDataUploadCnt);
	if (OverLoad_Mark)
	{
		printf("Buffer OVERLOAD(!!!)[%d]/[%d]\n", OverLoad_Mark, DataInputALLCount);
	}
	if (ModeChange_Mark == 1)
	{
		printf("\n\nSTART\n\n");
	}
	else if (ModeChange_Mark == 2)
	{
		printf("\n\nSTOP\n\n");
	}
	ModeChange_Mark = 0;
	
	exmliner_printStatus(0);

	return 0;
}
static int run_UploadData_Lthread(struct  lthread * self)
{
	if (AccData.len > 0)
	{
		AccADataUploadLen += AccData.len;
		exmliner_Upload(&AccData, AccData.len, EXLNK_DATA_ID_DATA, 7);
		exlnk_clearData(&AccData);
	}
	if (GyrData.len > 0)
	{
		GyrDataUploadLen += GyrData.len;
		exmliner_Upload(&GyrData, GyrData.len, EXLNK_DATA_ID_DATA, 7);
		exlnk_clearData(&GyrData);
	}
	if (BccBData.len > 0)
	{
		BccBDataUploadLen += BccBData.len;
		exmliner_Upload(&BccBData, BccBData.len, EXLNK_DATA_ID_DATA, 7);
		exlnk_clearData(&BccBData);
	}
	return 0;
}

int applyExactolinkCommand( )
{
	lthread_launch(&UploadDataThread);

	if (Command_Mark != EXACTOLINK_NO_DATA)
	{
		if (Command_Mark == 5)
		{
			exSnsStart(EXACTOLINK_SNS_XL_0100_XLGR_0100);
			ModeChange_Mark = 1;
		}
		else if (Command_Mark == 9)
		{
			exSnsStop();
			ModeChange_Mark = 2;
		}
		else
		{
			// printf("\n\nUnknown command: %d\n\n", Command_Mark);
		}
		
		Command_Mark = 0;
	}
	return 0;	
}


int printSensorData ()
{
	if (Print_Mark)
	{
		printf("[%d]sensor:"
			"[%8d %8d %8d] "
			"[%8d %8d %8d] "
			"[%8d %8d %8d]"
			"\n"
			, Print_ItCounter++, 
			Print_Buffer[0], Print_Buffer[1], Print_Buffer[2],
			Print_Buffer[3], Print_Buffer[4], Print_Buffer[5],
			Print_Buffer[6], Print_Buffer[7], Print_Buffer[8]
		);
		// printf("Cmd mark: %d\n", Command_Mark);
		Acc_Mark = 1;
		Gyr_Mark = 1;
		GXL_Mark = 1;
		Print_Mark = 0;
	}
	return 0;	
}

int onUpdateSensorData(uint8_t * data, uint16_t len, uint8_t id)
{
	SensorDataUploadCnt += 1;
	DataInputALLCount += (uint32_t)len;
	uint16_t overload_value = 0;
	len = 6;
	if (id == LSM303AH)
	{
		overload_value = exlnk_addNewData(&AccData,len, data);
	}
	else if (id == ISM330DLC)
	{
		overload_value = exlnk_addNewData(&GyrData,len, data);
	}
	else if (id == ISM330DLC_XL)
	{
		overload_value = exlnk_addNewData(&BccBData,len, data);
	}
	if (overload_value > 0)
	{
		OverLoad_Mark += overload_value;
	}
	if (Print_Mark)
		return 0;
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
		if((!Print_Mark))
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
static int onCmdEventHandler(exlnk_cmd_str_t * cmd)
{
	Command_Mark = cmd->value;
	printf("Get Command:[reg: %3d val: %3d]\n", cmd->reg, cmd->value);
	// cmd->value += 3;
	// exmliner_Upload(cmd, sizeof(exlnk_cmd_str_t), EXLNK_DATA_ID_CMD, Address);
	SendCounter++;
	return 0;
}
static int onCmdAckEventHandler(exlnk_cmdack_str_t * cmd)
{
	printf("ack:[mnum: %5d reg: %5d]\n", cmd->mnum, cmd->reg);
	return 0;
}
static int onResetEventHandler()
{
	printf("Try reset Mline\n");
	return 0;
}
static int onRepeatEventHandler(uint8_t id, uint32_t mnum)
{
	printf("repeat: [%d %d ]\n", id, mnum);
	return 0;
}
static int onErrorEventHandler(int id)
{
	if(id == 1)
	{
		printf("Transmit failed\n");
	}
	else if (id == 2)
	{
		printf("Repeat failed\n");
	}
	else
	{
		printf("Unknown error\n");
	}
	return 0 ;
}
int main(int argc, char *argv[]) 
{
	printf("Basic application\nV 0.1\nAbilities:\n- Send ack\n- Change mode(start/stop sns)\n\n");
	lthread_init(&UploadDataThread, run_UploadData_Lthread);
	exmliner_setCmdAction(onCmdEventHandler);
	exmliner_setResetAction(onResetEventHandler);
	exmliner_setCmdAckAction(onCmdAckEventHandler);
	exmliner_setRepeatAction(onRepeatEventHandler);
	exmliner_setErrorAction(onErrorEventHandler);
	
	exmliner_init(&LsmIsmDev, onUpdateSensorData);

#ifdef MEASURE_TIME
	ex_dwt_cyccnt_reset();
	exutils_init(&TagTimer);
#endif

	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0, Address);

	exlnk_setData( & AccData, 33, ACCA_DATA_BUFFER_LEN, AccADataBuffer);
	exlnk_setData( &GyrData, 34, GYR_DATA_BUFFER_LEN, GyrDataBuffer);
	exlnk_setData( & BccBData, 35, BCCB_DATA_BUFFER_LEN, BccBDataBuffer);

	while (1)
	{
		while(!EnableUpdate)
			__asm("nop");
#ifdef MEASURE_TIME
		exutils_updt(&TagTimer);
#endif
		exmliner_Update(Address);
#ifdef MEASURE_TIME
		exutils_updt(&TagTimer);
		UpdateMlineDuration = TagTimer.result;
#endif
		int index = 0;
		while(!exmliner_getRxIRQ())
		{
			index++;
			if(index > 500000)
			{
				exmliner_Update(Address);
				index = 0;
				printf("reset irq\n");
				__asm("nop");
			}
		}
#ifdef MEASURE_TIME
		exutils_updt(&TagTimer);
		TransmitMlineDuration = TagTimer.result;
		UpdateMlineDurationAVR += UpdateMlineDuration;
		TransmitMlineDurationAVR += TransmitMlineDuration;
#endif
		printSensorData();
		applyExactolinkCommand();
		ReportStatus();
		if(NeedToPrint)
		{

#ifdef MEASURE_TIME
			UpdateMlineDurationAVR = UpdateMlineDurationAVR / TIM_1SEC_DIVIDER;
			TransmitMlineDurationAVR = TransmitMlineDurationAVR /TIM_1SEC_DIVIDER;
			printf("Update [%8d] Transmit [%8d]\n", UpdateMlineDurationAVR, TransmitMlineDurationAVR);
#endif
			printf("tim[%8d]send[%5d]\n", TIM_Counter,SendCounter);
			NeedToPrint = 0;
#ifdef MEASURE_TIME
			UpdateMlineDurationAVR = 0;
			TransmitMlineDurationAVR = 0;
#endif
		}
		EnableUpdate = 0;
		// sleep(1);
	}
	
	return 1;
}