#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "spi/spi1.h"

int main(int argc, char *argv[]) {
    printf("Disabling spi\n");
    disableMasterSpiDma();
    return 0;
}
