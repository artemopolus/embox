#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "spi/spi2_generated.h"
#include "gpio/gpio.h"

int main(int argc, char *argv[]) {
    printf("Enabling spi\n");
    // setupSPI2_FULL_DMA();
    while(1)
    {
        usleep(10000);
        if (ex_checkGpio())
        {
            break;
        }
    }
    printf("Done\n");
    
    return 0;
}
