#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <operator/exacto_buffer.h>
#define TEST_BFT_SZ 512
#define TEST_BFT_TRY_CNT 50

ExactoBufferUint8Type datastorage;
ExactoBufferExtended bigdatastorage;

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

int main(int argc, char *argv[]) {
    // uint8_t buffer_str[EXACTO_BUFFER_UINT8_SZ] = {0};
    // uint8_t buffer_dst[EXACTO_BUFFER_UINT8_SZ] = {0};

    uint8_t buffer_pck_src[TEST_BFT_SZ] = {0};
    uint8_t buffer_pck_dst[TEST_BFT_SZ] = {0};
    for (uint16_t i = 0; i < TEST_BFT_SZ; i++)
    {
       buffer_pck_src[i] = TEST_BFT_SZ - i; 
    }
    setini_exbextu8(&bigdatastorage);
    
    printf("Extended buffers test:\n");
    
    for (uint16_t i = 0; i < TEST_BFT_TRY_CNT; i++)
    {
        for(uint16_t j = 0; j < TEST_BFT_SZ; j++)
        {
            pshfrc_exbextu8(&bigdatastorage, buffer_pck_src[j]);
        }
    }

    for (uint16_t i = 0; i < TEST_BFT_TRY_CNT; i++)
    {
        for(uint16_t j = 0; j < TEST_BFT_SZ; j++)
        {
            uint8_t value;
            grbfst_exbextu8(&bigdatastorage, &value);
            buffer_pck_dst[j] = value;
        }
        printf("Check data [%d]:", i);
        if (checkBuffers(buffer_pck_dst, buffer_pck_src, TEST_BFT_SZ))
        {
            printf("Done\n");
        }
        else
        {
            printf("Failed\n");
        }
    }

    printf("Standart buffers test:\n");
    setini_exbu8(&datastorage);
    
    for (uint16_t i = 0; i < TEST_BFT_TRY_CNT; i++)
    {
        for(uint16_t j = 0; j < TEST_BFT_SZ; j++)
        {
            pshfrc_exbu8(&datastorage, buffer_pck_src[j]);
        }
    }

    for (uint16_t i = 0; i < TEST_BFT_TRY_CNT; i++)
    {
        for(uint16_t j = 0; j < TEST_BFT_SZ; j++)
        {
            uint8_t value;
            grbfst_exbu8(&datastorage, &value);
            buffer_pck_dst[j] = value;
        }
        printf("Check data [%d]:", i);
        if (checkBuffers(buffer_pck_dst, buffer_pck_src, TEST_BFT_SZ))
        {
            printf("Done\n");
        }
        else
        {
            printf("Failed\n");
        }
    }

    return 0;
}
