#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "tim/tim.h"

int main(int argc, char *argv[]) {

    uint8_t test_buffer[] = "hello there plz\n";
    ex_setData_ExactoDtStr(test_buffer, sizeof(test_buffer) , 1, EX_XL_LSM303AH );
    ex_setData_ExactoDtStr(test_buffer, sizeof(test_buffer) , 2, EX_XL_LSM303AH );
    ex_setData_ExactoDtStr(test_buffer, sizeof(test_buffer) , 3, EX_XL_LSM303AH );
    transmitExactoDataStorage();
    printf("Done\n");
    return 0;
}
