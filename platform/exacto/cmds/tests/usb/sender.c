#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "usbd_cdc_if.h"

uint8_t Message[] = "Hello there\n";

static uint32_t Index = 0;

static uint8_t recever(uint8_t* Buf, uint32_t Len)
{
	return 0 ;
}
int main(int argc, char *argv[]) 
{
	CDC_setReceiver_FS(recever);
	printf("Start sending via USB \n");
	while (1)
	{
		sleep(1);
		printf("Iter[%d]\n", Index);
		CDC_Transmit_FS(Message, sizeof(Message));
	}
	
	return 0;
}
