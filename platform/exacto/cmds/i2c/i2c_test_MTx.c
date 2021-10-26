#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "i2c/i2c_mliner.h"

static uint8_t Marker = 0;


static int transmitHandler(struct  lthread * self)
{
    Marker = 1;
    return 0;
}





int main(int argc, char *argv[]) {
    uint16_t a = 0;
    const uint16_t a_max = 1000;
    Marker = 0;
    printf("Start program: i2c master sends message\n");

    bind_TX_thread_I2C_MLINER(transmitHandler);

    enable_I2C_MLINER();

    while(a < a_max)
    {
        usleep(1000);
        if (Marker)
        {
            printf("Message was successfully transmited");
            break;
        }
        a++;
    }



    printf("Program reached the end\n");

    return 0;
}

