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
#include "spi_mliner.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "commander/exacto_data_storage.h"
#include "ex_utils.h"

#include "gpio/gpio.h"

// #define SAM_REPORTER
#ifdef SAM_REPORTER
#include "kernel/printk.h"
uint32_t    SAM_Ticker_Start, 
            SAM_Ticker_Stop, 
            SAM_Ticker_Result;
#endif


#define SPI2_FULL_DMA_RXTX_BUFFER_SIZE SPI_MLINER_BUFFER_SIZE
typedef struct
{
    uint8_t dt_buffer[SPI2_FULL_DMA_RXTX_BUFFER_SIZE];
    uint16_t dt_count;
    struct mutex dt_mutex;
    struct lthread dt_lth;
    uint8_t is_full;
} SPI2_FULL_DMA_buffer;

// static uint8_t PtExactoStorage = 0xFF;

static SPI2_FULL_DMA_buffer SPI2_FULL_DMA_rx_buffer = {
    .dt_count = SPI2_FULL_DMA_RXTX_BUFFER_SIZE,
    .is_full = 0,
};
static SPI2_FULL_DMA_buffer SPI2_FULL_DMA_tx_buffer = {
    .dt_count = SPI2_FULL_DMA_RXTX_BUFFER_SIZE,
    .is_full = 0,
};
static irq_return_t SPI2_FULL_DMA_tx_irq_handler(unsigned int irq_nr, void *data);
static irq_return_t SPI2_FULL_DMA_rx_irq_handler(unsigned int irq_nr, void *data);
static int SPI2_FULL_DMA_rx_handler(struct lthread *self);
// static int SPI2_FULL_DMA_tx_handler(struct lthread *self);
static int SPI2_FULL_DMA_rx_handler(struct lthread *self);
static int SPI2_FULL_DMA_tx_handler(struct lthread *self);

static int SPI2_FULL_DMA_transmit(struct lthread * self);
static int SPI2_FULL_DMA_receive(struct lthread * self);

EMBOX_UNIT_INIT(SPI2_FULL_DMA_init);
static int SPI2_FULL_DMA_init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  /**SPI2 GPIO Configuration
  PB13   ------> SPI2_SCK
  PB14   ------> SPI2_MISO
  PB15   ------> SPI2_MOSI
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* SPI2 DMA Init */

  /* SPI2_RX Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MDATAALIGN_BYTE);

  /* SPI2_TX Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MDATAALIGN_BYTE);

    /* SPI2 interrupt Init */

    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_SLAVE;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 10;
    LL_SPI_Init(SPI2, &SPI_InitStruct);

    // DMA settings
    // DMA_CHANNEL_4 -> RX
    // DMA_CHANNEL_5 -> TX
    LL_DMA_ConfigAddresses(DMA1,
                            LL_DMA_CHANNEL_4,
                            LL_SPI_DMA_GetRegAddr(SPI2), (uint32_t)SPI2_FULL_DMA_rx_buffer.dt_buffer,
                            LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_rx_buffer.dt_count);
    LL_DMA_ConfigAddresses(DMA1,
                            LL_DMA_CHANNEL_5, (uint32_t)SPI2_FULL_DMA_tx_buffer.dt_buffer,
                            LL_SPI_DMA_GetRegAddr(SPI2),
                            LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, SPI2_FULL_DMA_tx_buffer.dt_count);


    // DMA interrupts
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_5);

    /* embox specific section  */
    // embox DMA interrupts handlers
    irq_attach(15, SPI2_FULL_DMA_tx_irq_handler, 0, NULL, "SPI2_FULL_DMA_TX_irq_handler");
    irq_attach(14, SPI2_FULL_DMA_rx_irq_handler, 0, NULL, "SPI2_FULL_DMA_RX_irq_handler");

    // init lthread for receive and transmit events
    lthread_init(&SPI2_FULL_DMA_tx_buffer.dt_lth, &SPI2_FULL_DMA_tx_handler);
    lthread_init(&SPI2_FULL_DMA_rx_buffer.dt_lth, &SPI2_FULL_DMA_rx_handler);

    //init lthread for middle level driver named exacto_data_storage
    lthread_init(&ExDtStr_Output_Storage[EX_THR_SPi_TX].thread, &SPI2_FULL_DMA_transmit);
    ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 1;
    lthread_init(&ExDtStr_Output_Storage[EX_THR_SPi_RX].thread, &SPI2_FULL_DMA_receive);
    ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 0;
    /* embox specific section  */
    //enable hardware for SPI and DMA
    // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
    // LL_SPI_EnableDMAReq_RX(SPI2);
    // LL_SPI_EnableDMAReq_TX(SPI2);
    // LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);   //transmit
    // LL_SPI_Enable(SPI2);
#ifdef SAM_REPORTER
    ex_dwt_cyccnt_reset();
#endif
    return 0;
}
extern void setupSpiReceiveSlave()
{
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_Enable(SPI2);
}
void setupSPI2_FULL_DMA()
{
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);   //transmit
    LL_SPI_Enable(SPI2);

}
void turnOffSPI2_FULL_DMA()
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5);
    LL_SPI_Disable(SPI2);
    LL_SPI_DisableDMAReq_RX(SPI2);
    LL_SPI_DisableDMAReq_TX(SPI2);
}
static irq_return_t SPI2_FULL_DMA_tx_irq_handler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC5(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_GI5(DMA1);
        lthread_launch(&SPI2_FULL_DMA_tx_buffer.dt_lth);
    }
    EDS_spidmairq_Marker = 1;
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(15, SPI2_FULL_DMA_tx_irq_handler, NULL);
static irq_return_t SPI2_FULL_DMA_rx_irq_handler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC4(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_GI4(DMA1);
        lthread_launch(&SPI2_FULL_DMA_rx_buffer.dt_lth);
    }
    EDS_spidmairq_Marker = 1;
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(14, SPI2_FULL_DMA_rx_irq_handler, NULL);
static int SPI2_FULL_DMA_rx_handler(struct lthread *self)
{
    goto *lthread_resume(self, &&start);
start:
    /* инициализация */
mutex_retry:
    if (mutex_trylock_lthread(self, &ExDtStorage.dtmutex ) == -EAGAIN)
    {
        return lthread_yield(&&start, &&mutex_retry);
    }
    ExDtStorage.isEmpty = 0;
    SPI2_FULL_DMA_rx_buffer.is_full = 1;
    ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 1;
    ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_WAIT;
    ex_updateCounter_ExDtStr(EX_THR_SPi_RX);
    mutex_unlock_lthread(self, &ExDtStorage.dtmutex);

    return 0;
}
static int SPI2_FULL_DMA_tx_handler(struct lthread *self)
{
    SPI2_FULL_DMA_tx_buffer.is_full = 0;
    ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 1;
    //ExDtStr_Output_Storage[EX_THR_SPi_TX].result = EX_THR_CTRL_NO_RESULT;
    // ex_updateCounter_ExDtStr(EX_THR_SPi_TX);
#ifdef SAM_REPORTER
    SAM_Ticker_Stop = ex_dwt_cyccnt_stop();
    SAM_Ticker_Result = SAM_Ticker_Stop - SAM_Ticker_Start;
    printk("%d",SAM_Ticker_Result);
#endif
    return 0;
}
void SPI2_disableChannels()
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
}
void SPI2_enableChannels()
{
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
}
void SPI2_updateRx()
{
    if (ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)
    {
        for (uint16_t i = 0; i < SPI2_FULL_DMA_RXTX_BUFFER_SIZE; i++)
            pshfrc_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage, SPI2_FULL_DMA_rx_buffer.dt_buffer[i]);
        ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 0;
    }
}
static int SPI2_FULL_DMA_transmit(struct lthread * self)
{
#ifdef SAM_REPORTER
    SAM_Ticker_Start = ex_dwt_cyccnt_start();
#endif
    exactolink_package_result_t res;
    ex_getExactolinkType(&res);
    if (res == EXACTOLINK_NO_DATA)
    {
        if (!ExDtStr_Output_Storage[EX_THR_SPi_RX].isready&&!ex_checkGpio(EX_GPIO_SPI_MLINE))  //можно ли обновлять
        {
            LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
            LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
            LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
        }
        return 1;
    }
    if  (
            (ExDtStr_Output_Storage[EX_THR_SPi_RX].result == EX_THR_CTRL_OK)
        )
    {
        ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_NO_RESULT;
    }


    if (
        (ExDtStr_Output_Storage[EX_THR_SPi_TX].result == EX_THR_CTRL_OK)
        &&(ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
        )
    {
        //данные готовы к отправке и шлюз свободен
        if (!ex_checkGpio(EX_GPIO_SPI_MLINE))  //можно ли обновлять
        {
            SPI2_disableChannels();
            getMailFromExactoDataStorage(SPI2_FULL_DMA_tx_buffer.dt_buffer, SPI2_FULL_DMA_tx_buffer.dt_count);
            ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 0;
            ExDtStr_Output_Storage[EX_THR_SPi_TX].result = EX_THR_CTRL_INIT;
            ex_updateCounter_ExDtStr(EX_THR_SPi_TX);
            SPI2_updateRx();
            SPI2_enableChannels();
        }
    }
    else if (
        (ExDtStr_Output_Storage[EX_THR_SPi_TX].result != EX_THR_CTRL_OK)
        &&(ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
        )
    {
        // данные не готовы, но шлюз свободен : ничего не делать, только принимать что-либо
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
        SPI2_updateRx();
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
    }
    else // ! ExDtStr_Output_Storage[EX_THR_SPi_TX].isready
    {
        //шлюз занят, готовность данных не имеет значения : повторить отправку
        if (!ex_checkGpio(EX_GPIO_SPI_MLINE))  //можно ли обновлять
        {
            SPI2_disableChannels();
            SPI2_updateRx();
            SPI2_enableChannels();
        }
    }
    return 0;
}
static int SPI2_FULL_DMA_receive(struct lthread * self)
{
    ex_thread_control_t * _trg_thread;
    _trg_thread = (ex_thread_control_t *)self;
    const uint32_t _datacount = SPI2_FULL_DMA_rx_buffer.dt_count ;
    if (SPI2_FULL_DMA_rx_buffer.is_full == 0)
        return 1;
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    for (uint16_t i = 0; i < _datacount; i++)
        pshfrc_exbu8(&_trg_thread->datastorage, SPI2_FULL_DMA_rx_buffer.dt_buffer[i]);
    SPI2_FULL_DMA_rx_buffer.is_full = 0;
    _trg_thread->isready = 0;
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, _datacount);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    return 0;
}
uint8_t SPI2_FULL_DMA_setdatalength( uint8_t datalength )
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    SPI2_FULL_DMA_rx_buffer.dt_count = datalength;
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, datalength);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
    return 0;
}
struct mutex SPI2_FULL_DMA_wait_rx_data(void)
{
    return SPI2_FULL_DMA_rx_buffer.dt_mutex;
}
uint8_t SPI2_FULL_DMA_is_full(void)
{
    return SPI2_FULL_DMA_rx_buffer.is_full;
}
