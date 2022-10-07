#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "gpio/gpio.h"

int main(int argc, char *argv[]) {
    usleep(1000000);
    ex_enableLed(EX_LED_GREEN);
    //usleep(1000000);
    //ex_enableLed(EX_LED_BLUE);
 
    return 0;
}

