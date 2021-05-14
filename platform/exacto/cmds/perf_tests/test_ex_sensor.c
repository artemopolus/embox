#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <util/err.h>

#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/task.h>
#include <kernel/time/ktime.h>
#include <kernel/printk.h>



#include <sys/wait.h>
#include "tim/tim.h"
#include "ex_utils.h"

#include "commander/exacto_data_storage.h"
#include "commander/exacto_sns_ctrl.h"
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"
#include "spi/spi_sns.h"


static ex_spi_pack_t TES_PackageToSend = {
    .result = EXACTO_OK,
};
static ex_spi_pack_t TES_PackageToGett = {
    .result = EXACTO_WAITING,
};
exacto_sensors_list_t TES_CurTrgSens = LSM303AH;
uint8_t TES_CurTrgSens_isenabled  = 0;
static struct lthread TES_Send_Lthread;
ex_service_transport_msg_t TES_BufferToData;

void tes_sendOptions(exacto_sensors_list_t sns, const uint8_t address, const uint8_t value)
{
    TES_PackageToSend.data[0] = address;
    TES_PackageToSend.data[1] = value;
    TES_PackageToSend.datalen = 2;
    TES_PackageToSend.type = EX_SPI_DT_TRANSMIT;
    TES_CurTrgSens = sns;
    lthread_launch(&TES_Send_Lthread);
}
void tes_sendAndReceive(exacto_sensors_list_t sns, const uint8_t address, const uint8_t datalen)
{
    TES_PackageToSend.result = EXACTO_WAITING;
    TES_PackageToSend.type = EX_SPI_DT_TRANSMIT_RECEIVE;
    TES_PackageToGett.cmd = address ;
    TES_PackageToGett.datalen = datalen;
    TES_CurTrgSens = sns;
    lthread_launch(&TES_Send_Lthread);
}
static int runTES_Send_Lthread(struct lthread * self)
{
    TES_CurTrgSens_isenabled = 1;
    enableExactoSensor(TES_CurTrgSens);
    if (TES_PackageToSend.type == EX_SPI_DT_TRANSMIT)
    {
        ex_sendSpiSns(&TES_PackageToSend);
    }
    else if (TES_PackageToSend.type == EX_SPI_DT_TRANSMIT_RECEIVE)
    {
        ex_gettSpiSns(&TES_PackageToGett);
    }
    disableExactoSensor(TES_CurTrgSens);
    return 0;
}
void tes_printReceivedData()
{
    // printf("\033[A\33[2K\r");
    printf("Get some data: ");
    for (uint8_t i = 0; i < TES_PackageToGett.datalen; i++)
    {
        uint8_t ctrl_value = TES_PackageToGett.data[i];
        printf(" %d %#04x| ", ctrl_value, ctrl_value);
    }
    printf("\n");
    // printf("Counter: %d\n", SensorTickerCounter);
}

int main(int argc, char *argv[]) {
    printf("Start testing sensors\n");
    ex_initServiceMsg(&TES_BufferToData);
    lthread_init(&TES_Send_Lthread, runTES_Send_Lthread);
    tes_sendOptions(LSM303AH, LSM303AH_3WIRE_ADR, LSM303AH_3WIRE_VAL);
    tes_sendOptions(ISM330DLC, ISM330DLC_CTRL3_C, 0x0c);
    tes_sendAndReceive(LSM303AH, LSM303AH_WHOAMI_XL_ADR, 2);
    tes_printReceivedData();
    printf("Done\n");
    return 0;
}
