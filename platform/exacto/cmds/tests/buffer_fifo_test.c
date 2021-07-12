#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <commander/exacto_buffer.h>
#include <kernel/lthread/lthread.h>

static struct lthread Basic; 
    ExactoBufferUint8Type datastorage;
    uint8_t DoneMarker = 0;

void setDataToBuffer(ExactoBufferUint8Type * buffer, uint8_t * src, const uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        pshfrc_exbu8(buffer, src[i]);
    }
}
void getDataFromBuffer(ExactoBufferUint8Type * buffer, uint8_t * src, const uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t value;
        grbfst_exbu8(buffer, &value);
        src[i] = value;
    }
}
uint8_t checkBuffers(uint8_t * dst, uint8_t * src, const uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        if (dst[i] != src[i])
        {
            return 0;
        }
    }
    return 1;
}
static int runBasicLthread( struct lthread * self)
{
    setini_exbu8(&datastorage);
    DoneMarker = 1;
    return 0;
}
int main(int argc, char *argv[]) {
    uint8_t buffer_str[EXACTO_BUFFER_UINT8_SZ] = {0};
    uint8_t buffer_dst[EXACTO_BUFFER_UINT8_SZ] = {0};
    lthread_init(&Basic, runBasicLthread);
    lthread_launch(&Basic);
    while (!DoneMarker)
    {
    }
    

    for (uint16_t i = 0; i < EXACTO_BUFFER_UINT8_SZ; i++)
    {
        uint8_t value = (uint8_t) i;
        buffer_str[i] = value;
        // pshfrc_exbu8(&datastorage, value);
    }
    uint16_t size = 45;
    for (uint8_t i = 0; i < 3; i++)
    {
        setDataToBuffer(&datastorage, buffer_str, size);
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        getDataFromBuffer(&datastorage, buffer_dst, size);
        printf("Check data [%d]:", i);
        if (checkBuffers(buffer_str, buffer_dst, size))
        {
            printf("Done\n");
        }
        else
        {
            printf("Failed\n");
        }
    }
    
    printf("Datalen: %d\n", getlen_exbu8(&datastorage));
    return 0;
}
