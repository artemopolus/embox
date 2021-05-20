#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "tim/tim.h"

int main(int argc, char *argv[]) {

    uint8_t test_buffer[] = "hello there plz\n";
    setDataToExactoDataStorage(test_buffer, sizeof(test_buffer), EXACTO_INIT);
    transmitExactoDataStorage();
    printf("Done\n");
    return 0;
}
