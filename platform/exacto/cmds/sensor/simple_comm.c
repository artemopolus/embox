#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>

#include <commander/exacto_sensors.h>
#include <commander/exacto_data_storage.h>
#include "spi/spi1_generated.h"


struct lthread RxCheckThread;
struct lthread TxCheckThread;
struct lthread UploadDataThread;


static spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};

uint8_t MarkerRx = 0;
uint8_t MarkerTx = 0;

void updatePackageToSend(uint8_t * data, const uint8_t datalen)
{
    for (uint8_t i = 0; (i < datalen)&&(i < SPI_PACK_SZ); i++)
    {
        PackageToSend.data[i] = data[i];
    }
    PackageToSend.result = EXACTO_OK;
    PackageToSend.type = SPI_DT_TRANSMIT;
}

static int runTxCheckThread( struct lthread * self)
{
    if (PackageToSend.result == EXACTO_OK)
    {
        MarkerTx = 1;
    }
    
    return 0;
}
static int runRxCheckThread(struct lthread * self)
{
    if (PackageToGett.result == EXACTO_OK)
    {
        MarkerRx = 1;
    }
    return 0;
}

static int runUploadDataThread(struct lthread * self)
{
    setDataToExactoDataStorage(PackageToGett.data, PackageToGett.datalen);
    return 0;
}

uint8_t setOpt( const uint8_t value, const uint8_t address)
{
    if(MarkerTx)
        return 1;
    PackageToSend.data[0] = address & 0x7F;
    PackageToSend.data[1] = value;
    PackageToSend.datalen = 2;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT;
    sendSpi1Half(&PackageToSend);
    MarkerTx = 1;
    return 0;
}
uint8_t getData( const uint8_t address)
{
    if(MarkerTx)
        return 1;
    PackageToSend.data[0] = address | 0x80;
    PackageToSend.datalen = 1;
    PackageToSend.result = EXACTO_WAITING;
    PackageToSend.type = SPI_DT_TRANSMIT_RECEIVE;
    sendSpi1Half(&PackageToSend);
    MarkerTx = 1;
    return 0;
}
uint8_t receiveData( )
{
    if (!MarkerRx)
    {
        waitSpi1Half(&PackageToGett);
        return 1;
    }
    else
    {
        // lthread_launch(&UploadDataThread);
        MarkerRx = 0;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    lthread_init(&TxCheckThread, runTxCheckThread);
    lthread_init(&RxCheckThread, runRxCheckThread);
    lthread_init(&UploadDataThread, runUploadDataThread);

    // const uint8_t adr_mask = 0x7F;

    const uint8_t lsm303ah_3wire_adr = 0x21;
    const uint8_t lsm303ah_3wire_val = 0x05;
    const uint8_t lsm303ah_whoami_xl_adr = 0x0f;
    setOpt(lsm303ah_3wire_val,lsm303ah_3wire_adr);

    while (MarkerTx)
    {
        lthread_launch(&TxCheckThread);
    }

    getData(lsm303ah_whoami_xl_adr);
        // const uint8_t lsm303ah_whoami_xl_val = 0x43;

    // const uint8_t lsm303ah_whoami_mg_adr = 0x4f;
    // const uint8_t lsm303ah_whoami_mg_val = 0x40;

    // //0x12, 0x0C

    // const uint8_t ism330dlc_3wire_adr = 0x12;
    // const uint8_t ism330dlc_3wire_val = 0x0C;

    // const uint8_t ism330dlc_whoami_adr = 0x0F;
    // const uint8_t ism330dlc_whoami_val = 0x6A;
    return 0;
}