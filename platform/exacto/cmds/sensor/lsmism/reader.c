#include "sensors/lsmism.h"

static uint16_t Counter = 0;
static uint16_t MaxCounter = 100;

static uint8_t PrintMark = 0;

static int16_t AccBuffer[3] = {0};

static uint32_t ItCounter = 0;

int onUpdateSensorData(uint8_t * data, uint16_t len, uint8_t id)
{
	if(Counter > MaxCounter)
	{
		Counter = 0;
		if(!PrintMark)
		{
			if(id == LSM303AH )
			{
				for(int i = 0; i < 3; i++)
					exlnk_cv_Uint8_Int16(&data[i*2], (int16_t *)&AccBuffer[i]);
			}
			PrintMark  = 1;
		}
	}
	else
		Counter++;
	return 0;	
}

int main(int argc, char *argv[]) 
{
	printf("Start sensor lsmism reader\n");
	exmliner_init(&LsmIsmDev, onUpdateSensorData);

	exSnsStart(EXACTOLINK_SNS_XL_0100_XLGR_0100);
	while(1)
	{
		if(PrintMark)
		{
			printf("[%d]sensor:[%8d %8d %8d]\n", ItCounter++, AccBuffer[0], AccBuffer[1], AccBuffer[2]);
			PrintMark = 0;
		}
	}
	return 0;
}