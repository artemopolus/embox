#include "mliner/mliner_main.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"
#include <stdio.h>

#include "ex_utils.h"
#include "sensors/lsmism.h"
#include "exactolink/exlnk_Data.h"
#include "exactolink/exlnk_Status.h"

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

// ===
struct lthread UploadDataThread[4];


#define TIM_1SEC_DIVIDER 200
#define MEASURE_TIME

#ifdef MEASURE_TIME
static exutils_data_t TagTimer;
static uint32_t 	
						UploadToMlineDuration = 0,
						UpdateMlineDuration = 0,
						UpdateMlineDurationAVR = 0,
						TransmitMlineDurationAVR = 0
						;
#endif
static uint8_t TransmitMlineDuration_meas_on = 0;
static uint32_t TransmitMlineDuration = 0;
static uint8_t TransmitMlineDuration_cnt = 0;

static uint32_t Meas2MeasTime_duration = 0;
static uint8_t Meas2MeasTime_meas_on = 0;
static uint8_t Meas2MeasTime_cnt = 0;

static uint8_t UploadToMlineEventCnt = 0;

static exlnk_status_str_t StatusData;

static uint16_t TIM_Counter = 0;
static uint16_t SendCounter = 0;

static int PointToTim;
static uint8_t EnableUpdate = 0;
static uint8_t ReadyToUpload = 0;
static uint8_t NeedToPrint = 0;

static uint8_t Address = 7;

static exlnk_data_str_t AccData;
static exlnk_data_str_t GyrData;
static exlnk_data_str_t BccBData;

// static uint16_t TIM_Event_Counter = 0;

int ReportStatus()
{
	if (NeedToPrint == 0)
		return 1;

	if (1)
	{

		printf("Tim Events: %d\n", StatusData.tim_event_counter);
		printf("ACC upload[%d]\n", StatusData.acc_a_data_upload_len );
		printf("GYR upload[%d]\n", StatusData.gyr_data_upload_len );
		printf("BCC upload[%d]\n", StatusData.bcc_b_data_upload_len);
		printf("SNS Events: %d\n", StatusData.sensor_data_upload_cnt );
		if (StatusData.over_load_mark)
		{
			printf("Buffer OVERLOAD(!!!)[%d]/[%d]\n", StatusData.over_load_mark, StatusData.data_input_all_count );
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
	}

	
	exmliner_printStatus(0);

	return 0;
}
uint16_t uploadDataPackToExMliner( exlnk_data_str_t * trg)
{
	#define SEC_MODULE_DATA_TO_COPY 30
	if (exlnk_getDataLen(trg) > SEC_MODULE_DATA_TO_COPY)
	{
		exmliner_Upload(trg, 72, EXLNK_DATA_ID_DATA, 7);
		return trg->latest_upload;
	}
	return 0;
}
static int run_MlinerPack_Lthread(struct  lthread * self)
{
	exmliner_checkHeaders();
	return 0;
}
static int run_UploadInfo_Lthread(struct  lthread * self)
{
	if (exmliner_checkStatus(Address)) return 0;
	exmliner_checkHeaders();
	exmliner_UploadALLACKS(7);
	exmliner_Upload(&StatusData,0,EXLNK_DATA_ID_STATUS, 7);
	return 0;
}
static uint32_t getTagTimerResult()
{
	exutils_updt(&TagTimer);
	uint32_t duration = TagTimer.result;
	if (TransmitMlineDuration_meas_on)
		TransmitMlineDuration += duration;
	if (Meas2MeasTime_meas_on)
		Meas2MeasTime_duration += duration;
	return duration;
}
static int run_UploadData_Lthread(struct  lthread * self)
{
	if (exmliner_checkStatus(Address)) return 0;

#ifdef MEASURE_TIME
	getTagTimerResult();
#endif	
	exmliner_checkHeaders();
	StatusData.acc_a_data_upload_len += uploadDataPackToExMliner( &AccData );
	StatusData.gyr_data_upload_len += uploadDataPackToExMliner ( &GyrData );
	StatusData.bcc_b_data_upload_len += uploadDataPackToExMliner( &BccBData);

#ifdef MEASURE_TIME
	uint32_t duration = getTagTimerResult();
	UploadToMlineDuration += duration;
	UploadToMlineEventCnt ++;
#endif	
	return 0;
}

int applyExactolinkCommand( )
{
	lthread_launch(&UploadDataThread[2]);
	lthread_launch(&UploadDataThread[0]);
	lthread_launch(&UploadDataThread[1]);

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
	Meas2MeasTime_meas_on = 0;
	getTagTimerResult();
	StatusData.sensor_data_upload_cnt += 1;
	StatusData.data_input_all_count += (uint32_t)len;
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
		StatusData.over_load_mark += overload_value;
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

	if (Meas2MeasTime_cnt < 32)
		Meas2MeasTime_cnt++;
	else
	{
		StatusData.sns2sns_time = Meas2MeasTime_duration /Meas2MeasTime_cnt;
		Meas2MeasTime_duration = 0;
		Meas2MeasTime_cnt = 0;
	}
	getTagTimerResult();
	Meas2MeasTime_meas_on = 1;

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
			StatusData.tim_event_counter ++;
		}
	}
	ReadyToUpload = 1;
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
static int run_Transmit_Lthread(struct  lthread * self)
{
	exmliner_gogoTransmit();
	return 0;
}
static int onTransmitHandler(uint16_t adr)
{
	lthread_launch(&UploadDataThread[3]);
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
	printf("Basic application\nV 0.2\n"
		"Abilities:\n"
		"- Send Ack\n"
		"- Send Status\n"
		"- Change mode(start/stop sns)\n"
		"\n");
	lthread_init(&UploadDataThread[0], run_UploadData_Lthread);
	lthread_init(&UploadDataThread[1], run_UploadInfo_Lthread);
	lthread_init(&UploadDataThread[2], run_MlinerPack_Lthread);
	lthread_init(&UploadDataThread[3], run_Transmit_Lthread);
	exmliner_setCmdAction(onCmdEventHandler);
	exmliner_setResetAction(onResetEventHandler);
	exmliner_setCmdAckAction(onCmdAckEventHandler);
	exmliner_setRepeatAction(onRepeatEventHandler);
	exmliner_setErrorAction(onErrorEventHandler);
	exmliner_seTransmit(onTransmitHandler);

	printf("Init sensors\n");
	exmliner_init(&LsmIsmDev, onUpdateSensorData);

#ifdef MEASURE_TIME
	ex_dwt_cyccnt_reset();
	exutils_init(&TagTimer);
#endif

	printf("Init timers\n");
	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0, Address);

	printf("Init buffers for data\n");

	exlnk_setData( & AccData, 33);
	exlnk_setData( & GyrData, 34);
	exlnk_setData( & BccBData, 35);
	exlnk_status_init(&StatusData);

	printf("Start basic cycle\n");
	uint32_t index = 0;

	while (1)
	{
		uint8_t receive_marker = exmliner_getRxIRQ();
		if (ReadyToUpload && receive_marker)
		{
			applyExactolinkCommand();
		}
		if (!EnableUpdate)
		{
			__asm("nop");
		}
		else
		{
			if (!receive_marker)
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
			else
			{

#ifdef MEASURE_TIME
				TransmitMlineDuration += getTagTimerResult();
				TransmitMlineDuration_meas_on = 0;
				TransmitMlineDurationAVR += TransmitMlineDuration;
				StatusData.mline_sns_upload_duration = UploadToMlineDuration / UploadToMlineEventCnt;

				UploadToMlineDuration = 0;
				UploadToMlineEventCnt = 0;
#endif


				exmliner_Update(Address);
#ifdef MEASURE_TIME
				UpdateMlineDuration = getTagTimerResult();
				UpdateMlineDurationAVR += UpdateMlineDuration;
#endif

				printSensorData();
				ReportStatus();
				if(NeedToPrint)
				{

#ifdef MEASURE_TIME
					UpdateMlineDurationAVR = UpdateMlineDurationAVR / TIM_1SEC_DIVIDER;
					// TransmitMlineDurationAVR = TransmitMlineDurationAVR /TIM_1SEC_DIVIDER;
					printf("Update [%8d] Transmit [%8d]\n", UpdateMlineDurationAVR, StatusData.mline_wait_duration);
#endif
					printf("tim[%8d]send[%5d]\n", TIM_Counter,SendCounter);
					NeedToPrint = 0;
#ifdef MEASURE_TIME
					UpdateMlineDurationAVR = 0;
#endif
				}
				EnableUpdate = 0;
#ifdef MEASURE_TIME
				getTagTimerResult();
				if (TransmitMlineDuration_cnt < 32)
					TransmitMlineDuration_cnt++;
				else
				{
					StatusData.mline_wait_duration = TransmitMlineDurationAVR / TransmitMlineDuration_cnt;
					TransmitMlineDuration_cnt = 0;
					TransmitMlineDurationAVR = 0;
				}
				TransmitMlineDuration = 0;
				TransmitMlineDuration_meas_on = 1;
#endif
			}
		// sleep(1);
		}
	}
	
	return 1;
}