#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <commander/exacto_buffer.h>


int main(int argc, char *argv[]) {
    ExactoBufferUint8Type datastorage;
    uint8_t buffer[64] = {0};
    setini_exbu8(&datastorage);

    for (uint8_t i = 0; i < 16; i++)
    {
        pshfrc_exbu8(&datastorage, i);
        printf("lst: %d\n", datastorage.lst);
    }
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    for (uint8_t i = 0; i < 5; i++)
    {
        pshfrc_exbu8(&datastorage, i);
        printf("lst: %d\n", datastorage.lst);
    }
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    for (uint8_t i = 0; i < 4; i++)
    {
        pshfrc_exbu8(&datastorage, i);
        printf("lst: %d\n", datastorage.lst);
    }    
    printf("Data:lst: %d\n", datastorage.lst);
    
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    grball_exbu8(&datastorage, buffer);

    printf("Data:lst: %d\n", datastorage.lst);
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));

    for (uint8_t i = 0; i < 64; i++)
    {
        printf("%d ", buffer[i]);
    }

    setemp_exbu8(&datastorage);

    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    return 0;
}
