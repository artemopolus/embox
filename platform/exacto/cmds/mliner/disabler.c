#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "spi/spi2_generated.h"

int main(int argc, char *argv[]) {
    printf("Disabling spi\n");
    turnOffSPI2_FULL_DMA();
    return 0;
}
