#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_gpio.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "spi_sns.h"
#include "kernel/printk.h"


ex_subs_service_t ExSnsServices[EX_SPI_SERVICES_COUNT];
ex_service_info_t ExSnsServicesInfo = {
  .max_count = EX_SPI_SERVICES_COUNT,
  .current_count = 0,
};



EMBOX_UNIT_INIT(initSpi1HalfDMA);
static int initSpi1HalfDMA(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    SPI_InitStruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 10;
    LL_SPI_Init(SPI1, &SPI_InitStruct);

    LL_SPI_Enable(SPI1);
    return 0;
}

void checkChannelsForDisable()
{
    
}
// static int txSpi1HalfRun(struct lthread *self)
uint8_t ex_runReceiver()
{

    return 0;
}
uint8_t ex_runTransmiter()
{
    return 0;
}

#define SPI_APPOLON_INDEX_MAX 1000
#define SPI_APPOLON_INDEX_SZ_INT uint16_t

uint8_t ex_sendSpiSns(ex_spi_pack_t * input)
{
    const uint8_t address = input->data[0];
    const uint8_t value = input->data[1];
    uint8_t mask = 0x7F ;//01111111b
	mask &= address;
    // remember to reset CS -->LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
	// LL_SPI_SetTransferDirection(SPI1,LL_SPI_HALF_DUPLEX_TX);
    // LL_SPI_Enable(SPI1);
    SPI_APPOLON_INDEX_SZ_INT i = 0;
    i = 0;
    while(!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
            return 1;
    }
    // for (SPI_APPOLON_INDEX_SZ_INT i = 0; !LL_SPI_IsActiveFlag_TXE(SPI1);i++)
        // if (i > SPI_APPOLON_INDEX_MAX) return 1;
	// while(LL_SPI_IsActiveFlag_BSY(SPI1));
    LL_SPI_TransmitData8(SPI1, mask);
    i = 0;
    while(!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
            return 1;
    }
	LL_SPI_TransmitData8(SPI1, value);
    i = 0;
	while(!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
            return 1;
    }
    i = 0;
	while(LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
            return 1;
    }
    // LL_SPI_Disable(SPI1);
	// remember to set CS -->LL_GPIO_SetOutputPin(GPIOA,LL_GPIO_PIN_4);
    return 0;
}
uint8_t __attribute__((optimize("O0")))ex_gettSpiSns(ex_spi_pack_t *output)
{
	ipl_t ipl;

	ipl = ipl_save();

    EDS_spidmairq_Marker = 0;
    const uint8_t address = output->cmd;
    uint8_t value = address | 0x80;
    static uint8_t result = 0;
	LL_SPI_TransmitData8(SPI1, value);
    SPI_APPOLON_INDEX_SZ_INT i = 0;
	while(!LL_SPI_IsActiveFlag_TXE(SPI1))
    {
        if (i++ > SPI_APPOLON_INDEX_MAX)
        {
            result = 1;
            break;
        }
    }
    i = 0;
	while(LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
        {
            result = 1;
            break;
        }
    }
	LL_SPI_SetTransferDirection(SPI1,LL_SPI_HALF_DUPLEX_RX);
    for (uint8_t i = 0; i < output->datalen; i++)
    {
	    // while(!LL_SPI_IsActiveFlag_RXNE(SPI1));
        for(SPI_APPOLON_INDEX_SZ_INT j = 0; (!result) && (!LL_SPI_IsActiveFlag_RXNE(SPI1)); j++)
            result = (j > SPI_APPOLON_INDEX_MAX)? 1 : 0; 
        if (result)
            output->data[i] = 0;
        else
            output->data[i] = LL_SPI_ReceiveData8(SPI1);
    }
    i = 0;
	while(LL_SPI_IsActiveFlag_BSY(SPI1))
    {
        i++;
        if (i > SPI_APPOLON_INDEX_MAX)
        {
            result = 1;
            break;
        }
    }
	LL_SPI_SetTransferDirection(SPI1,LL_SPI_HALF_DUPLEX_TX);
    // if (LL_SPI_IsActiveFlag_RXNE(SPI1))
    // {
        for(SPI_APPOLON_INDEX_SZ_INT j = 0; ((!LL_SPI_IsActiveFlag_RXNE(SPI1))&&(j < SPI_APPOLON_INDEX_MAX)); j++)
            ;
            output->data[output->datalen] = LL_SPI_ReceiveData8(SPI1);
    // }
    // i = 0;
	// while(!LL_SPI_IsActiveFlag_RXNE(SPI1))
    // {
    //     if (i++ > SPI_APPOLON_INDEX_MAX)
    //     {
    //         result = 1;
    //         break;
    //     }
    // }
    // output->data[output->datalen - 1] = LL_SPI_ReceiveData8(SPI1);
    if (EDS_spidmairq_Marker)
        result = 1;
	ipl_restore(ipl);
    return result;
}
