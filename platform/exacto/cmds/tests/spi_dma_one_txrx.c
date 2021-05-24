#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "spi/spi_mliner.h"
#include "gpio/gpio.h"

#define MAX_CALL_COUNT 10


#define TRANSMIT_MESSAGE_SIZE 64



uint8_t DataToBuffer[TRANSMIT_MESSAGE_SIZE] = {0};
uint8_t ReceivedData[TRANSMIT_MESSAGE_SIZE] = { 0};




struct lthread PrintDataFromBufferThread;




static int printBufferData(struct  lthread * self)
{
    printf("Received buffer: ");
    for (uint8_t i = 0; i < TRANSMIT_MESSAGE_SIZE; i++)
    {
        printf("%d ", ReceivedData[i]);
    }
    printf("\n");
    return 0;
}





int main(int argc, char *argv[]) {
    printf("Start Full Duplex SPI\n");
    ex_enableGpio();
    enableMasterSpiDma(); 
    lthread_init(&PrintDataFromBufferThread, printBufferData);
    printf("Reset ALL\n");
    // resetExactoDataStorage();
    printf("Init buffer: \n-upload\n");
    printf("Upload data to buffer\n");
    printf("Data[ buffer] = > SPI[TX]\n");
    printf("Run cycle for checking:\n");
    printf("Try \n" );

    uint8_t a = 0;
    while(a < 10)
    {
            usleep(1000000);


        setDataToExactoDataStorage(DataToBuffer, TRANSMIT_MESSAGE_SIZE, THR_CTRL_OK); 
        transmitExactoDataStorage();
        printf("Tx\n");
        receiveExactoDataStorage();
        printf("Download data from data storage\n");
        getDataFromExactoDataStorage(ReceivedData, TRANSMIT_MESSAGE_SIZE);
    
    
        lthread_launch(&PrintDataFromBufferThread);
        a++;
    }

    uint8_t rx_pin = 0, tx_pin = 0;


    rx_pin = checkRxGetter();
    tx_pin = checkTxSender();

    printf("Tx: %d Rx: %d \n", tx_pin, rx_pin);


    printf("Programm reach end\n");
    ex_disableGpio();
    disableMasterSpiDma();

    return 0;
}

