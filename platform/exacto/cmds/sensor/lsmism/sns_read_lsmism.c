#include "sensors/sns_lsmism.h"



int main(int argc, char *argv[]) 
{
    uint8_t index = 0;
    while(index < 10)
    {
        Apollon_lsmism_MlineRXTx_Readable = 0;
        Apollon_lsmism_MlineRXTx_Readable = 0;
        Apollon_lsmism_Buffer_Readable = 0;
        //printf("Start read\n");
        while(!Apollon_lsmism_MlineRXTx_Readable);
        while (!Apollon_lsmism_Ticker_Readable);
        while(Apollon_lsmism_Buffer_Readable != 2);

        printf("[%d]Tk: %d OvrFw: %d RX %d|=%d|%d\t%d\t%d|=%d|%d\t%d\t%d|=|%d\t%d\t%d|\n",index++,
                                            Apollon_lsmism_Ticker_Buf, Apollon_lsmism_MlineOverFlow, Apollon_lsmism_MlineReceive
                                            ,Apollon_lsmism_Buffer_dtrd0
                                            ,Apollon_lsmism_Buffer_Data0[0], Apollon_lsmism_Buffer_Data0[1], Apollon_lsmism_Buffer_Data0[2]
                                            ,Apollon_lsmism_Buffer_dtrd1
                                            ,Apollon_lsmism_Buffer_Data1[0], Apollon_lsmism_Buffer_Data1[1], Apollon_lsmism_Buffer_Data1[2]
                                            ,Apollon_lsmism_Buffer_Data2[0], Apollon_lsmism_Buffer_Data2[1], Apollon_lsmism_Buffer_Data2[2]
                                            );
        Apollon_lsmism_MlineRXTx_Readable = 0;
        Apollon_lsmism_MlineRXTx_Readable = 0;
        sleep(1);
    }
    printf("Done\n");
    sleep(1);
    return 0;
}
