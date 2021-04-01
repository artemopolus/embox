#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "spi/spi1.h"
#include "gpio/gpio.h"

#define MAX_CALL_COUNT 10


#define TRANSMIT_MESSAGE_SIZE 16



uint8_t DataToBuffer[] = {5, 7, 2, 3, 1,
                          0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0};
uint8_t ReceivedData[TRANSMIT_MESSAGE_SIZE] = { 0};




struct lthread PrintDataFromBufferThread;




static int printBufferData(struct  lthread * self)
{
    printf("Received buffer: ");
    for (uint8_t i = 0; i < 16; i++)
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


        setDataToExactoDataStorage(DataToBuffer, 16); 
        transmitExactoDataStorage();
        printf("Tx\n");
        receiveExactoDataStorage();
        printf("Download data from data storage\n");
        getDataFromExactoDataStorage(ReceivedData, 16);
    
    
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

