#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>

#include <commander/exacto_sensors.h>
#include <commander/exacto_data_storage.h>

struct lthread RxCheckThread;
struct lthread TxCheckThread;

static spi_pack_t PackageToSend = {
    .result = EXACTO_OK,
};
static spi_pack_t PackageToGett = {
    .result = EXACTO_WAITING,
};

uint8_t Marker = 0;

void updatePackageToSend(uint8_t * data, const uint8_t datalen)
{
    for (uint8_t i = 0; (i < datalen)&&(i < SPI_PACK_SZ); i++)
    {
        PackageToSend.data[i] = data[i];
    }
    PackageToSend.result = EXACTO_OK;
    PackageToSend.type = SPI_DT_TRANSMIT;
}

static int checkDataFromSend( struct lthread * self)
{
    if (PackageToSend.result == EXACTO_OK)
    {
        Marker = 1;
    }
    
    return 0;
}
static int checkDataFromGett(struct lthread * self)
{
    if (PackageToGett.result == EXACTO_OK)
    {
        Marker = 1;
    }
    return 0;
}

void sendCommand( const uint8_t value, const uint8_t address)
{

}
void receiveData( const uin8_t address, uint8_t * buffer, const u_int8_t buffer_len)
{

}

int main(int argc, char *argv[]) {

    const uint8_t adr_mask = 0x7F;

    const uint8_t lsm303ah_3wire_adr = 0x21;
    const uint8_t lsm303ah_3wire_val = 0x05;
    const uint8_t lsm303ah_whoami_xl_adr = 0x0f;
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