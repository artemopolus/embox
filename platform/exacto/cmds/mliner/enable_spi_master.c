#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "spi/spi1.h"
#include "gpio/gpio.h"

int main(int argc, char *argv[]) {
    printf("Enabling spi\n");
    ex_enableGpio();
    printf("Done\n");
    // enableMasterSpiDma();
    
    return 0;
}
