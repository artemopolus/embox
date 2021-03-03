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
#include "spi1_generated.h"

#define SPI1_HALF_BUFFER_SIZE 12
typedef struct
{
    struct lthread thread;
    uint8_t data[SPI1_HALF_BUFFER_SIZE];
    uint8_t datalen;
    // struct mutex mutex;
    uint8_t datapt;
    uint8_t overflow;
    spi_data_type_t type;
    spi_pack_t * buffer;
    exacto_process_result_t result;
} spi1_half_dma_buffer_t;

static spi1_half_dma_buffer_t TxSPI1HalfBuffer = {
    .datalen = SPI1_HALF_BUFFER_SIZE,
    .datapt = 0,
    .overflow = 0,
    .type = SPI_DT_TRANSMIT,
    .result = EXACTO_OK,
};
static spi1_half_dma_buffer_t RxSPI1HalfBuffer = {
    .datalen = 3, //SPI1_HALF_BUFFER_SIZE,
    .datapt = 0,
    .overflow = 0,
    .type = SPI_DT_RECEIVE,
    .result = EXACTO_WAITING,
};

struct lthread RxSpi1HalfIrqThread;
struct lthread TxSpi1HalfIrqThread;
struct lthread InitRxBufferThread;
struct lthread InitTxBufferThread;

//irq
static irq_return_t runRxSp1HalfDmaHandler(unsigned int irq_nr, void *data);
static irq_return_t runTxSp1HalfDmaHandler(unsigned int irq_nr, void *data);

static int txSpi1HalfRun(struct lthread *self);
static int rxSpi1HalfRun(struct lthread *self);
static int syncSpi1HalfRun(struct lthread * self);
static int initRxBuffer(struct lthread *self);
static int initTxBuffer(struct lthread * self);
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

    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_LOW);

    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);

    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);

    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);

    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);

    /* SPI1_TX Init */
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);

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
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
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
    lthread_init(&TxSpi1HalfIrqThread, txSpi1HalfRun);
    lthread_init(&RxSpi1HalfIrqThread, rxSpi1HalfRun);
    // lthread_init(&TxSpi1HalfRunThread, updateTxRun);
    lthread_init(&TxSPI1HalfBuffer.thread, syncSpi1HalfRun);
    lthread_init(&RxSPI1HalfBuffer.thread, syncSpi1HalfRun);
    lthread_init(&InitRxBufferThread, initRxBuffer);
    lthread_init(&InitTxBufferThread, initTxBuffer);

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
        lthread_launch(&RxSpi1HalfIrqThread);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(12, runRxSp1HalfDmaHandler, NULL);
static irq_return_t runTxSp1HalfDmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC3(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_GI3(DMA1);
        lthread_launch(&TxSpi1HalfIrqThread);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(13, runTxSp1HalfDmaHandler, NULL);
uint8_t SPI1_HALF_BASE_get_option(const uint8_t address)
{
    uint8_t value = address | 0x80;
	// remember to reset CS --> LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
	while(!LL_SPI_IsActiveFlag_TXE(SPI1));
	LL_SPI_TransmitData8(SPI1, value);
	while(!LL_SPI_IsActiveFlag_TXE(SPI1));
	while(LL_SPI_IsActiveFlag_BSY(SPI1));
	LL_SPI_SetTransferDirection(SPI1,LL_SPI_HALF_DUPLEX_RX);
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1));
	uint8_t result = 0;
	result = LL_SPI_ReceiveData8(SPI1);
	LL_SPI_SetTransferDirection(SPI1,LL_SPI_HALF_DUPLEX_TX);
	// remember to set CS --> LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);
    return result;
}
uint8_t SPI1_HALF_BASE_set_option(const uint8_t address, const uint8_t value)
{
    uint8_t mask = 0x7F ;//01111111b
	mask &= address;
    // remember to reset CS -->LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
    while(!LL_SPI_IsActiveFlag_TXE(SPI1));
    LL_SPI_TransmitData8(SPI1, mask);
    while(!LL_SPI_IsActiveFlag_TXE(SPI1));
	LL_SPI_TransmitData8(SPI1, value);
	while(!LL_SPI_IsActiveFlag_TXE(SPI1));
	while(LL_SPI_IsActiveFlag_BSY(SPI1));
	// remember to set CS -->LL_GPIO_SetOutputPin(GPIOA,LL_GPIO_PIN_4);
    return 0;
}
static int txSpi1HalfRun(struct lthread *self)
{
    TxSPI1HalfBuffer.result = EXACTO_OK;
    if (TxSPI1HalfBuffer.type == SPI_DT_TRANSMIT_RECEIVE)
    {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_RX);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2); // receive
    }
    return 0;
}
static int rxSpi1HalfRun(struct lthread *self)
{
    RxSPI1HalfBuffer.result = EXACTO_OK;
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        // LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, RxSPI1HalfBuffer.datalen);
        // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2); // receive
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit 
    return 0;
}
void initSpi1HalfBuffer(spi1_half_dma_buffer_t * buffer)
{
    buffer->datapt = 0;
    if (buffer->type == SPI_DT_TRANSMIT)
        buffer->result = EXACTO_OK;
    if (buffer->type == SPI_DT_RECEIVE)
        buffer->result = EXACTO_WAITING;
    for (uint8_t i = 0; i < buffer->datalen; i++)
    {
        buffer->data[i] = 0;
    }
    
}
static int initRxBuffer(struct lthread *self)
{
   initSpi1HalfBuffer(&RxSPI1HalfBuffer);
   return 0; 
}
static int initTxBuffer(struct lthread * self)
{
    initSpi1HalfBuffer(&TxSPI1HalfBuffer);
    return 0;
}
void set2receiveDMA(spi1_half_dma_buffer_t *output)
{
    // uint8_t tr = 0;
    uint8_t rc = 0;

    //----------------------------------
    switch (output->buffer->type)
    {
    case SPI_DT_RECEIVE:
        if (RxSPI1HalfBuffer.result == EXACTO_OK)
        {
            // if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_3)) //transmit
            // {
            //     LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
            //     tr = 1;
            // }
            if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2)) //receive
            {
                LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
                rc = 1;
            }
            // LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
            for (uint8_t i = 0; i < output->datalen; i++)
            {
                output->buffer->data[i] = output->data[i];
            }
            output->buffer->result = EXACTO_OK;
            output->result = EXACTO_WAITING;
            for (uint8_t i = 0; i < output->datalen; i++)
            {
                output->data[i] = 0;
            }
            // if (tr)
            //     LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
            if (rc)
                LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
            // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
        }
        else
        {
            output->buffer->result = output->result;
        }

        break;
    case SPI_DT_SET:
        output->datalen = output->buffer->datalen;
        //LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, output->datalen);
        output->buffer->result = EXACTO_OK;
        break;
    
    default:
        break;
    }
    //----------------------------------

}
void data2transmitDMA(spi1_half_dma_buffer_t *input)
{
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_3)) //transmit
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2))
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    for (uint8_t i = 0; i < input->buffer->datalen; i++)
    {
        input->data[i] = input->buffer->data[i];
    }
    input->datalen = input->buffer->datalen;
    input->buffer->result = EXACTO_OK;  // operation success
    input->result = EXACTO_WAITING;     // wait to transmit data
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, input->datalen);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3); //transmit
    LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
}
void transmitDataHandler(spi1_half_dma_buffer_t * input)
{
    switch (input->buffer->type)
    {
    case SPI_DT_TRANSMIT_RECEIVE:
        input->type = SPI_DT_TRANSMIT_RECEIVE;
        data2transmitDMA(input);
        break;
    case SPI_DT_TRANSMIT:
        input->type = SPI_DT_TRANSMIT;
        data2transmitDMA(input);
        break;
    case SPI_DT_CHECK:
        input->buffer->result = input->result; 
        break;
    default:
        break;
    }
}
void receiveDataHandler(spi1_half_dma_buffer_t * output)
{
    switch (output->buffer->type)
    {
    case SPI_DT_RECEIVE:
    case SPI_DT_SET:
        set2receiveDMA(output);
        break;
    case SPI_DT_CHECK:
        output->buffer->result = output->result;
        break;
    
    default:
        break;
    }
}
static int syncSpi1HalfRun(struct lthread * self)
{
    spi1_half_dma_buffer_t * _trg;
    _trg = (spi1_half_dma_buffer_t * ) self;
    goto *lthread_resume(self, &&start);
start:
    /* инициализация */
mutex_retry:
    if (mutex_trylock_lthread(self, &ExDtStorage.dtmutex ) == -EAGAIN)
    {
        return lthread_yield(&&start, &&mutex_retry);
    }
   //disable interrupts for SPI
    if ((_trg->type == SPI_DT_TRANSMIT)||(_trg->type == SPI_DT_TRANSMIT_RECEIVE))
    {
        transmitDataHandler(_trg);
    }
    else if (_trg->type == SPI_DT_RECEIVE)
    {
        receiveDataHandler(_trg);
    }
    
    if (_trg->datapt != 0)
    {
        ExDtStorage.isEmpty = 0;
    }
    mutex_unlock_lthread(self, &ExDtStorage.dtmutex);

    return 0;
}
uint8_t sendSpi1Half(spi_pack_t * input)
{
    if (input->datalen > SPI1_HALF_BUFFER_SIZE)
        return 1;
    TxSPI1HalfBuffer.buffer = input;
    lthread_launch(&TxSPI1HalfBuffer.thread);
    return 0;
}
uint8_t waitSpi1Half(spi_pack_t *output)
{
    // if (output-> datalen <= SPI1_HALF_BUFFER_SIZE)
        // return 1;
    RxSPI1HalfBuffer.buffer = output;
    lthread_launch(&RxSPI1HalfBuffer.thread);
    return 0;
}
