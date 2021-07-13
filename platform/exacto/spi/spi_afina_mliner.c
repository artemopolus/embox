#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx.h"
#include "stm32f7xx_ll_i2c.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_spi.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_cortex.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_pwr.h"
#include "spi_mliner.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/printk.h>
#include "commander/exacto_data_storage.h"


#include "gpio/gpio.h"

#define SAM_PACKAGE_START_POINTER 2
static uint16_t SAM_PackageStart_Buffer = SAM_PACKAGE_START_POINTER;

/**
 * @brief опредееляет количество данных, выделяемых в DMA
 * 
 */
#define SPI1_FULL_DMA_RXTX_BUFFER_SIZE SPI_MLINER_BUFFER_SIZE
/**
 * @brief стркутура для хранения легкого потока с дополнительными данными
 * 
 */
typedef struct
{
    uint8_t dt_buffer[SPI1_FULL_DMA_RXTX_BUFFER_SIZE];
    uint16_t dt_count;
    struct mutex dt_mutex;
    struct lthread dt_lth;
    uint8_t is_full;
} SPI1_FULL_DMA_buffer;
/**
 * @brief буффер для входящих данных
 * 
 */

static SPI1_FULL_DMA_buffer SPI1_FULL_DMA_rx_buffer = {
    .dt_count = SPI1_FULL_DMA_RXTX_BUFFER_SIZE,
    .is_full = 0,
};
/**
 * @brief буффер для исходящих данных
 * 
 */
static SPI1_FULL_DMA_buffer SPI1_FULL_DMA_tx_buffer = {
    .dt_count = SPI1_FULL_DMA_RXTX_BUFFER_SIZE,
    .is_full = 0,
};

static uint8_t SPI1_Sync_Marker = 0;

static irq_return_t SPI1_FULL_DMA_tx_irq_handler(unsigned int irq_nr, void *data);
static irq_return_t SPI1_FULL_DMA_rx_irq_handler(unsigned int irq_nr, void *data);
static int SPI1_FULL_DMA_rx_handler(struct lthread *self);
static int SPI1_FULL_DMA_tx_handler(struct lthread *self);

static int SPI1_FULL_DMA_enabled(struct lthread * self)
{
    LL_SPI_Enable(SPI1);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0); //enable receive
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
    return 0;

}

static int SPI1_FULL_DMA_transmit(struct lthread * self);
static int SPI1_FULL_DMA_receive(struct lthread * self);
EMBOX_UNIT_INIT(SPI1_FULL_DMA_init);
static int SPI1_FULL_DMA_init(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    /**SPI1 GPIO Configuration
     PA5   ------> SPI1_SCK
    PA7   ------> SPI1_MOSI
    PB4   ------> SPI1_MISO
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_RX Init */
    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);

      /* SPI1_TX Init */
    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_5, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_5, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_5);

    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
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
    // LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    // LL_SPI_EnableNSSPulseMgt(SPI1);

    // DMA settings
    // DMA_STREAM_0 -> RX
    // DMA_STREAM_5 -> TX
    LL_DMA_ConfigAddresses(DMA2, 
                            LL_DMA_STREAM_0,
                           LL_SPI_DMA_GetRegAddr(SPI1), (uint32_t)SPI1_FULL_DMA_rx_buffer.dt_buffer,
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_STREAM_0));
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, SPI1_FULL_DMA_rx_buffer.dt_count);

    LL_DMA_ConfigAddresses(DMA2,
                           LL_DMA_STREAM_5, (uint32_t)SPI1_FULL_DMA_tx_buffer.dt_buffer,
                           LL_SPI_DMA_GetRegAddr(SPI1),
                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_STREAM_5));
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_5, SPI1_FULL_DMA_tx_buffer.dt_count);

    // DMA interrupts
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_5);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_5);

    /* embox specific section  */
    // embox DMA interrupts handlers
    irq_attach(68, SPI1_FULL_DMA_tx_irq_handler, 0, NULL, "SPI1_FULL_DMA_irq_handler"); // like NVIC_SetPriority(DMA2_Stream5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    irq_attach(56, SPI1_FULL_DMA_rx_irq_handler, 0, NULL, "SPI1_FULL_DMA_irq_handler");

    // init lthread for receive and transmit events
    lthread_init(&SPI1_FULL_DMA_tx_buffer.dt_lth, &SPI1_FULL_DMA_tx_handler);
    lthread_init(&SPI1_FULL_DMA_rx_buffer.dt_lth, &SPI1_FULL_DMA_rx_handler);

    //init lthread for middle level driver named exacto_data_storage
    lthread_init(&ExDtStr_Output_Storage[EX_THR_SPi_TX].thread, &SPI1_FULL_DMA_transmit);
    ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 1;
    lthread_init(&ExDtStr_Output_Storage[EX_THR_SPi_RX].thread, &SPI1_FULL_DMA_receive);
    ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 0;

    lthread_init(&ExSpi.thread, &SPI1_FULL_DMA_enabled);
    ExSpi.isready = 1;
    /* embox specific section  */

    //enable hardware for SPI and DMA
    // LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0); //enable receive
    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_SPI_EnableDMAReq_TX(SPI1);
    // LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
    LL_SPI_Enable(SPI1);
    //SPI1_FULL_DMA_setdatalength(SPI1_FULL_DMA_RXTX_BUFFER_SIZE);
    return 0;
}
void enableMasterSpiDma()
{
    LL_DMA_SetDataLength    (DMA2, LL_DMA_STREAM_5, SPI1_FULL_DMA_RXTX_BUFFER_SIZE); //устанавливаем сколько символов передачть
    LL_DMA_SetDataLength    (DMA2, LL_DMA_CHANNEL_0, SPI1_FULL_DMA_RXTX_BUFFER_SIZE);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0); //enable receive
    // LL_SPI_EnableDMAReq_RX(SPI1);
    // LL_SPI_EnableDMAReq_TX(SPI1);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
    // LL_SPI_Enable(SPI1);

}
void disableMasterSpiDma()
{
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0); //enable receive
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
    // LL_SPI_Disable(SPI1);
    // LL_SPI_DisableDMAReq_RX(SPI1);
    // LL_SPI_DisableDMAReq_TX(SPI1);

}

/**
 * @brief Реакция на окончание передачи данных
 * 
 * @param irq_nr 
 * @param data 
 * @return irq_return_t 
 */
static irq_return_t SPI1_FULL_DMA_tx_irq_handler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC5(DMA2) != RESET)
    {
        //LL_DMA_IsActiveFlag_TC5(DMA2);
        LL_DMA_ClearFlag_TC5(DMA2);
        lthread_launch(&SPI1_FULL_DMA_tx_buffer.dt_lth);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(68, SPI1_FULL_DMA_tx_irq_handler, NULL);
/**
 * @brief Реакция на окончание приема данных
 * 
 * @param irq_nr 
 * @param data 
 * @return irq_return_t 
 */
static irq_return_t SPI1_FULL_DMA_rx_irq_handler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC0(DMA2) != RESET)
    {
        LL_DMA_ClearFlag_TC0(DMA2);
        lthread_launch(&SPI1_FULL_DMA_rx_buffer.dt_lth);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(56, SPI1_FULL_DMA_rx_irq_handler, NULL);
/**
 * @brief обновляем информацию о переданных данных
 * 
 * @param self 
 * @return int 
 */
static int SPI1_FULL_DMA_tx_handler(struct lthread *self)
{
    ex_updateCounter_ExDtStr(EX_THR_SPi_TX);
    SPI1_FULL_DMA_buffer * _trg_buffer;
    _trg_buffer = (SPI1_FULL_DMA_buffer*) self;
    _trg_buffer->is_full = 0;
    ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 1;
    ex_disableGpio(EX_GPIO_SPI_MLINE); //теперь можно обновлять tx на аполлоне
    return 0;
}
/**
 * @brief легкий поток для получения данных через DMA
 * 
 * @param self легкий поток 
 * @return int ноль при успешном завершении
 */
static int SPI1_FULL_DMA_rx_handler(struct lthread *self)
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
    mutex_unlock_lthread(self, &ExDtStorage.dtmutex);
    // receiveExactoDataStorage();
    if (SPI1_FULL_DMA_rx_buffer.dt_buffer[SAM_PACKAGE_START_POINTER] == EXACTOLINK_PCK_ID)
    {
        ex_updateCounter_ExDtStr(EX_THR_SPi_RX);
        ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 1;
        SAM_PackageStart_Buffer = SAM_PACKAGE_START_POINTER;
        return 0;
    }
    else
    {
        for (uint16_t i = 0; i < SPI1_FULL_DMA_RXTX_BUFFER_SIZE; i++)
        {
            if (SPI1_FULL_DMA_rx_buffer.dt_buffer[i] == EXACTOLINK_PCK_ID)
            {
                SAM_PackageStart_Buffer = i;
                ex_updateCounter_ExDtStr(EX_THR_SPi_RX);
                ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 1;
                return 0;
            }
        }
        
    }
    // printk("$");
    return 0;
}
void SPI1_updateTx()
{
    if (!ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
        return;
    uint8_t value;
    for (uint16_t i = 0; i < ExDtStr_Output_Storage[EX_THR_SPi_TX].datalen; i++)
    {
        grbfst_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_TX].datastorage, &value);
        SPI1_FULL_DMA_tx_buffer.dt_buffer[i] = value;
    }
    ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 0;
}

/**
 * @brief записываем данные в массив передачи данных
 * 
 * @param self 
 * @return int 
 */
static int SPI1_FULL_DMA_transmit(struct lthread * self)
{
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("*");
#endif
    if (SPI1_Sync_Marker)
    {
        ex_toggleGpio(EX_GPIO_SPI_SYNC);    
    }
    if ((ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)&&(ExDtStr_Output_Storage[EX_THR_SPi_RX].result == EX_THR_CTRL_OK))
    {
        //Данные пришли на вход и проверены
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("q");
#endif
        disableMasterSpiDma();
        setemp_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage);
        for (uint16_t i = SAM_PackageStart_Buffer; i < SPI1_FULL_DMA_RXTX_BUFFER_SIZE; i++)
        {
            uint8_t value;                        
            value = SPI1_FULL_DMA_rx_buffer.dt_buffer[i];
            // if ((i == SAM_PackageStart_Pointer)&&(value != 17))
                // printk("+");
            
            pshfrc_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage, value);
            // pshfrc_exbu8(&ExDtStr_Output_Storage[EX_THR_SPi_RX].datastorage, SPI1_FULL_DMA_rx_buffer.dt_buffer[i]);
        }
        ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 0;
        ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_WAIT;
        SPI1_updateTx();
        ex_enableGpio(EX_GPIO_SPI_MLINE); //block update apollon tx
        enableMasterSpiDma();
    }
    else if ((ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)&&(ExDtStr_Output_Storage[EX_THR_SPi_RX].result == EX_THR_CTRL_WAIT))
    {
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("w");
#endif
        // Данные приходили, но не проверены: ничего не делать
    }
    else if ((!ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)&&(ExDtStr_Output_Storage[EX_THR_SPi_RX].result == EX_THR_CTRL_NO_RESULT))
    {
        // Начальное состояние
        ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 1;
        ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_OK;
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("e");
#endif

    }
    else if (!ExDtStr_Output_Storage[EX_THR_SPi_RX].isready)
    {
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("r");
#endif
        // данные не приходили, состояние не интересует : перезагрузить отправку данных
        // LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
        // LL_DMA_SetDataLength    (DMA2, LL_DMA_STREAM_5, SPI1_FULL_DMA_RXTX_BUFFER_SIZE); //устанавливаем сколько символов передачть
        // LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5); //enable transmit 
        disableMasterSpiDma();
        SPI1_updateTx();
        ex_disableGpio(EX_GPIO_SPI_MLINE); //block update apollon tx
        enableMasterSpiDma();
    }
    else
    {
#ifdef PRINTK_ID_FOR_THREAD_ON
    printk("t");
#endif
        // Данные пришли, но состояние не известно : отправить на проверку
        ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_WAIT;
    }
    return 0;
}
/**
 * @brief забираем данные 
 * 
 * @param self 
 * @return int 
 */
static int SPI1_FULL_DMA_receive(struct lthread * self)
{
    ex_thread_control_t * _trg_thread;
    _trg_thread = (ex_thread_control_t *)self;
    if (!_trg_thread->isready)
        return 0;
    
    const uint32_t _datacount = SPI1_FULL_DMA_rx_buffer.dt_count;   //сколько данных влезает в буффер

    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);                    //отлючаем поток передачи данных
    for (uint8_t i = 2; i < _datacount; i++)                        //
        pshfrc_exbu8(&_trg_thread->datastorage, SPI1_FULL_DMA_rx_buffer.dt_buffer[i]);
    _trg_thread->isready = 0;
    LL_DMA_SetDataLength    (DMA2, LL_DMA_CHANNEL_0, _datacount);
    LL_DMA_EnableStream (DMA2, LL_DMA_STREAM_0);
    return 0;
}
uint8_t SPI1_FULL_DMA_setdatalength( uint8_t datalength )
{
    LL_DMA_DisableStream    (DMA2, LL_DMA_STREAM_0);
    SPI1_FULL_DMA_rx_buffer.dt_count = datalength;
    LL_DMA_SetDataLength    (DMA2, LL_DMA_CHANNEL_0, datalength);
    LL_DMA_EnableStream (DMA2, LL_DMA_STREAM_0);
    return 0;
}
struct mutex SPI1_FULL_DMA_wait_rx_data(void)
{
    return SPI1_FULL_DMA_rx_buffer.dt_mutex;
}
uint8_t SPI1_FULL_DMA_is_full(void)
{
    return SPI1_FULL_DMA_rx_buffer.is_full;
}
void syncMasterSpiDma()
{
    SPI1_Sync_Marker = 1;
}
