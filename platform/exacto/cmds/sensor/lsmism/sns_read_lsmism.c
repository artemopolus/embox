#include "sensors/sns_lsmism.h"



int main(int argc, char *argv[]) 
{
    uint8_t index = 0;
    while(1)
    {
        Apollon_lsmism_MlineRXTx_Readable = 0;
        Apollon_lsmism_MlineRXTx_Readable = 0;
        printf("Start read\n");
        while(!Apollon_lsmism_MlineRXTx_Readable);
        while (!Apollon_lsmism_Ticker_Readable);

        printf("[%d]Tk: %d Tx: %d RX %d\n",index++, Apollon_lsmism_Ticker_Buf, Apollon_lsmism_MlineTransmit, Apollon_lsmism_MlineReceive);
        Apollon_lsmism_MlineRXTx_Readable = 0;
        Apollon_lsmism_MlineRXTx_Readable = 0;
        sleep(1);
    }
    printf("Done");
    return 0;
}
