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


ex_subs_service_t ExSnsServices[EX_SPI_SERVICES_COUNT];
ex_service_info_t ExSnsServicesInfo = {
  .max_count = EX_SPI_SERVICES_COUNT,
  .current_count = 0,
};


// #define SPI1_HALF_BUFFER_SIZE 12
typedef struct
{
    struct lthread thread;
    // uint8_t data[SPI1_HALF_BUFFER_SIZE];
    uint8_t data[EX_SPI_PACK_SZ];
    uint8_t datalen;
    // struct mutex mutex;
    uint8_t datapt;
    uint8_t overflow;
    ex_spi_data_type_t type;
    ex_spi_pack_t * buffer;
    exacto_process_result_t result;
} spi1_half_dma_buffer_t;

static spi1_half_dma_buffer_t TxSPI1HalfBuffer = {
    .datalen = 3,
    .datapt = 0,
    .overflow = 0,
    .type = EX_SPI_DT_TRANSMIT,
    .result = EXACTO_OK,
};
static spi1_half_dma_buffer_t RxSPI1HalfBuffer = {
    .datalen = EX_SPI_PACK_SZ, //SPI1_HALF_BUFFER_SIZE,
    .datapt = 0,
    .overflow = 0,
    .type = EX_SPI_DT_RECEIVE,
    .result = EXACTO_WAITING,
};

struct lthread RxSpi1HalfIrqThread;
struct lthread TxSpi1HalfIrqThread;

//irq
static irq_return_t runRxSp1HalfDmaHandler(unsigned int irq_nr, void *data);
static irq_return_t runTxSp1HalfDmaHandler(unsigned int irq_nr, void *data);

EMBOX_UNIT_INIT(initSpi1HalfDMA);
static int initSpi1HalfDMA(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 DMA Init */

    /* SPI1_RX Init */
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);

    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);

    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);

    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);

    /* SPI1_TX Init */
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_HIGH);

    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);

    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);

    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);

    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

    /* SPI1 interrupt Init */

    SPI_InitStruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 10;
    LL_SPI_Init(SPI1, &SPI_InitStruct);

    // memory config
    LL_DMA_ConfigAddresses(DMA1, 
                         LL_DMA_CHANNEL_3,  //LL_DMA_DIRECTION_MEMORY_TO_PERIPH
                         (uint32_t)TxSPI1HalfBuffer.data, LL_SPI_DMA_GetRegAddr(SPI1),
                         LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, TxSPI1HalfBuffer.datalen);
    LL_DMA_ConfigAddresses(DMA1, 
                         LL_DMA_CHANNEL_2,//LL_DMA_DIRECTION_PERIPH_TO_MEMORY
                         LL_SPI_DMA_GetRegAddr(SPI1), (uint32_t)RxSPI1HalfBuffer.data,
                         LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);


    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
//   DMA1_Channel2_IRQn          = 12,     /*!< DMA1 Channel 2 global Interrupt                      */
//   DMA1_Channel3_IRQn          = 13,     /*!< DMA1 Channel 3 global Interrupt                      */
    irq_attach(12, runRxSp1HalfDmaHandler, 0, NULL, "Receive SPI1 Half Duplex DMA irq handler");
    irq_attach(13, runTxSp1HalfDmaHandler, 0, NULL, "Transmit SPI1 Half Duplex DMA irq handler");

    // lthread init
    // lthread_init(&TxSpi1HalfIrqThread, txSpi1HalfRun);
    // lthread_init(&RxSpi1HalfIrqThread, rxSpi1HalfRun);
    // lthread_init(&TxSpi1HalfRunThread, updateTxRun);
    
    
    
    ex_initSubscribeEvents( ExSnsServicesInfo, ExSnsServices);  

// irq init
    //enable spi and set transfer direction
    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_SPI_EnableDMAReq_TX(SPI1);
  //  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
  //  LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
    LL_SPI_Enable(SPI1);
    return 0;
}
static irq_return_t runRxSp1HalfDmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC2(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_GI2(DMA1);
        // lthread_launch(&RxSpi1HalfIrqThread);
        ex_updateEventForSubs(ExSnsServicesInfo, ExSnsServices, THR_SPI_RX);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(12, runRxSp1HalfDmaHandler, NULL);
static irq_return_t runTxSp1HalfDmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC3(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_GI3(DMA1);
        // lthread_launch(&TxSpi1HalfIrqThread);
        ex_updateEventForSubs(ExSnsServicesInfo, ExSnsServices, THR_SPI_TX);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(13, runTxSp1HalfDmaHandler, NULL);

void checkChannelsForDisable()
{
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_3)) //transmit
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2))
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

}
// static int txSpi1HalfRun(struct lthread *self)
uint8_t ex_runReceiver()
{
    // while(!LL_SPI_IsActiveFlag_TXE(SPI1)){}
    // while(LL_SPI_IsActiveFlag_BSY(SPI1)){}
    // LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
    // LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    // checkChannelsForDisable();
    // LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);
    // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2); // receive
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_RX);
    return 0;
}
uint8_t ex_runTransmiter()
{
    // LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    // LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    // checkChannelsForDisable();
    // LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
    // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit 
    return 0;
}
void initSpi1HalfBuffer(spi1_half_dma_buffer_t * buffer)
{
    buffer->datapt = 0;
    if (buffer->type == EX_SPI_DT_TRANSMIT)
        buffer->result = EXACTO_OK;
    if (buffer->type == EX_SPI_DT_RECEIVE)
        buffer->result = EXACTO_WAITING;
    for (uint8_t i = 0; i < buffer->datalen; i++)
    {
        buffer->data[i] = 0;
    }
    
}

uint8_t ex_sendSpiSns(ex_spi_pack_t * input)
{
    // checkChannelsForDisable();
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
    
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    for (uint8_t i = 0; i < input->datalen; i++)
    {
        TxSPI1HalfBuffer.data[i] = input->data[i];
    }
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, input->datalen);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
    
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2); // receive
    return 0;
}
uint8_t ex_gettSpiSns(ex_spi_pack_t *output)
{
    // if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2)) //receive
    // {
    //     LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    // }
    for (uint8_t i = 0; i < RxSPI1HalfBuffer.datalen; i++)
    {
        output->data[i] = RxSPI1HalfBuffer.data[i];
    }
    output->result = EXACTO_OK;
    output->datalen = RxSPI1HalfBuffer.datalen;
    return 0;
}
