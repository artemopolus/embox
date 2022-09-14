#include "mliner/mliner_sec.h"
#include <kernel/lthread/lthread.h>
#include "tim/tim.h"

static uint16_t TIM_Counter = 0;
static int PointToTim;
static uint8_t EnableUpdate = 0;
static uint8_t NeedToPrint = 0;


static int run_Tim_Lthread(struct  lthread * self)
{
	exse_ack(&ExTimServices[PointToTim]);
	if(TIM_Counter < 10000)
		TIM_Counter++;
	else
		TIM_Counter = 0;

	if(NeedToPrint == 0)
	{
		if(! (TIM_Counter % 200))
			NeedToPrint = 1;
	}
	EnableUpdate = 1;
	return 0;
}
int main(int argc, char *argv[]) 
{
	PointToTim = exse_subscribe(&ExTimServicesInfo, ExTimServices, EX_THR_TIM, run_Tim_Lthread);
	ex_setFreqHz(100);
	exmliner_Init(0);
	while (1)
	{
		while(!EnableUpdate);
		exmliner_Update();
		if(NeedToPrint)
		{

		}
		// sleep(1);
	}
	
	return 1;
}