#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "exacto_commander/exacto_data_storage.h"
#include "exacto_commander/exacto_sensors.h"
#include "project/base_project_defs.h"
#include "spi_gen/spi1_generated.h"

static spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};
typedef struct{
    struct lthread thread;
    uint8_t data[SPI_PACK_SZ];
    uint8_t datalen;
}thread_container_t;

uint8_t Marker = 0;

thread_container_t AppendDataToSendThread;
thread_container_t AppendDataToSendAndReceiveThread;
thread_container_t CheckDataFormGettThread;
thread_container_t CheckDataFromSendThread;

struct lthread SetPackageToGettToNullThread;

static int setPackageToGettToNull(struct lthread * self)
{
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        PackageToGett.data[i] = 0;
    }
    PackageToGett.result = EXACTO_WAITING;
    
    return 0;
}

static int appendDataToSensAndReceive(struct lthread *self)
{
    thread_container_t * _trg;
    _trg = (thread_container_t *) self;
    for (uint8_t i = 0; i < _trg->datalen; i++)
    {
        PackageToSend.data[i] = _trg->data[i];
    }
    PackageToSend.datalen = _trg->datalen;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT_RECEIVE;
    sendSpi1Half(&PackageToSend);

    return 0;
}

static int appendDataToSend(struct lthread * self)
{
    thread_container_t * _trg;
    _trg = (thread_container_t *)self;
    for (uint8_t i = 0; i < _trg->datalen; i++)
    {
        PackageToSend.data[i] = _trg->data[i];
    }
    PackageToSend.datalen = _trg->datalen;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT;
    sendSpi1Half(&PackageToSend);
    return  0;
}
static int checkDataFromSend( struct lthread * self)
{
    PackageToSend.type = SPI_DT_CHECK;
    if (PackageToSend.result == EXACTO_OK)
    {
        Marker = 1;
    }
    else{
        sendSpi1Half(&PackageToSend);
    }
    
    return 0;
}
static int checkDataFromGet(struct lthread * self)
{
    thread_container_t * _trg;
    _trg = (thread_container_t *)self;
    PackageToGett.type = SPI_DT_RECEIVE;
    PackageToGett.datalen = _trg->datalen;
    if (PackageToGett.result == EXACTO_OK)
    {
        Marker = 1;
        
    }
    else{
        waitSpi1Half(&PackageToGett);
    }
    return 0;
}

void sendAndReceive(exacto_sensors_list_t sns, const uint8_t address)
{
    AppendDataToSendAndReceiveThread.data[0] = address | 0x80;
    AppendDataToSendAndReceiveThread.datalen = 1;
    enableExactoSensor(sns);
    lthread_launch(&AppendDataToSendAndReceiveThread.thread);
    CheckDataFormGettThread.datalen = 3;
    Marker = 0;
    while(!Marker)
    {
        // sleep(1);
        lthread_launch(&CheckDataFromSendThread.thread);
    }
    lthread_launch(&SetPackageToGettToNullThread);
    Marker = 0;
    uint16_t counter = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFormGettThread.thread);
        counter++;
        if (counter > 1000)
        {
            printf("Failed\n");
            return;
        }
    }
    disableExactoSensor(sns);
    printf("Get some data: ");
    for (uint8_t i = 0; i < PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = PackageToGett.data[i];
        printf("[ %#04x = %d ]\t", ctrl_value, ctrl_value);
    }
    printf("\n");
    

}
void sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    AppendDataToSendThread.data[0] = address & 0x7F;
    AppendDataToSendThread.data[1] = value;
    AppendDataToSendThread.datalen = 2;

    enableExactoSensor(sns);
    lthread_launch(&AppendDataToSendThread.thread);
    Marker = 0;
    while(!Marker)
    {
        lthread_launch(&CheckDataFromSendThread.thread);
    }
    disableExactoSensor(sns);
 
}
int main(int argc, char *argv[]) {

    // const uint8_t adr_mask = 0x7F;

    const uint8_t lsm303ah_3wire_adr = 0x21;
    const uint8_t lsm303ah_3wire_val = 0x05;
    const uint8_t lsm303ah_whoami_xl_adr = 0x0f;
    // const uint8_t lsm303ah_whoami_xl_val = 0x43;

    const uint8_t lsm303ah_whoami_mg_adr = 0x4f;
    // const uint8_t lsm303ah_whoami_mg_val = 0x40; // or 41

    // //0x12, 0x0C

    const uint8_t ism330dlc_3wire_adr = 0x12;
    const uint8_t ism330dlc_3wire_val = 0x0C;

    const uint8_t ism330dlc_whoami_adr = 0x0F;
    // const uint8_t ism330dlc_whoami_val = 0x6A;

    // uint8_t data_mas[2] = {0};
    
    lthread_init(&SetPackageToGettToNullThread, setPackageToGettToNull);

    lthread_init(&AppendDataToSendThread.thread, appendDataToSend);
    lthread_init(&CheckDataFormGettThread.thread, checkDataFromGet);
    lthread_init(&CheckDataFromSendThread.thread, checkDataFromSend);
    lthread_init(&AppendDataToSendAndReceiveThread.thread, appendDataToSensAndReceive);

    printf("Start send data throw spi\n");
    sleep(1);
    sendOptions(LSM303AH, lsm303ah_3wire_adr, lsm303ah_3wire_val);
    printf("Options are sended!\n");
    printf("Wait whoami xl value\n");
    sleep(1);
    //check  xl data
    for (uint8_t i = 0; i < 9; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_whoami_xl_adr);
    }

    printf("Check whoami lsm303 mg value\n");
    sleep(1);
    //check mg data
    for (uint8_t i = 0; i < 9; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_whoami_mg_adr);
    }
    sleep(1);
    printf("Check whoami lsm303 xl and mg value\n");
    for (uint8_t i = 0; i < 4; i++)
    {
        sendAndReceive(LSM303AH, lsm303ah_whoami_xl_adr);
        sendAndReceive(LSM303AH, lsm303ah_whoami_mg_adr);
    }
    //set option for second sensor
    printf("Setup ism330 sensor\n");
    sendOptions(ISM330DLC, ism330dlc_3wire_adr, ism330dlc_3wire_val);
    printf("Check whoami lsm303 mg value\n");
    for (uint8_t i = 0; i < 9; i++)
    {
        sendAndReceive(ISM330DLC, ism330dlc_whoami_adr);
    }

    printf("Done\n");


    return 0;
}
