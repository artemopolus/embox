#include "mliner/mliner_maindev_impl.h"

#include "gpio_config.h"
#include "gpio/gpio_spi.h"

#include <embox/unit.h>
#include <kernel/irq.h>

#ifndef MLINE_RXTX_BUFFER_SIZE
#define MLINE_RXTX_BUFFER_SIZE 512
#endif

static uint8_t RxBuffer[MLINE_RXTX_BUFFER_SIZE];
static uint8_t TxBuffer[MLINE_RXTX_BUFFER_SIZE];

static uint32_t TxLen;
static uint32_t RxLen;

static struct mliner_dev * MlinerDev;
struct mliner_dev_ops SpiMainDev;

static irq_return_t TransmitIRQhandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC5(SPI_MLINE_DMA) != RESET)
    {
        LL_DMA_ClearFlag_TC5(SPI_MLINE_DMA);
        MlinerDev->readyTx = 1;
        ex_disableGpio(EX_GPIO_SPI_MLINE); //теперь можно обновлять tx на аполлоне
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(SPI_MLINE_DMA_IRQ_TX, TransmitIRQhandler, NULL);
static irq_return_t ReceiveIRQhandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC0(SPI_MLINE_DMA) != RESET)
    {
        LL_DMA_ClearFlag_TC0(SPI_MLINE_DMA);
        MlinerDev->readyRx = 1;
    }
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(SPI_MLINE_DMA_IRQ_RX, ReceiveIRQhandler, NULL);



static int updateRx(struct mliner_dev * dev)
{
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, RxLen); //устанавливаем сколько символов передачть
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    return 0;
}
static int updateTxRx(struct mliner_dev * dev)
{
    if(!dev->readyTx)
        return 0;
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    dev->receiveDataProcess(RxBuffer, RxLen);
    dev->transmitDataProcess(TxBuffer, TxLen);
    ex_enableGpio(EX_GPIO_SPI_MLINE); //block update apollon tx
    LL_DMA_SetDataLength     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, RxLen);
    LL_DMA_SetDataLength     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, TxLen); //устанавливаем сколько символов передачть

    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    dev->readyRx = 0;
    dev->readyTx = 0;
    return 1;
}
static int read(struct mliner_dev * dev, uint8_t * data, uint16_t len)
{
   return 0;
}
static int close (struct mliner_dev * dev)
{
    dev->is_opened = 0;
	irq_detach(SPI_MLINE_DMA_IRQ_TX,NULL);
	irq_detach(SPI_MLINE_DMA_IRQ_RX,NULL);
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    LL_SPI_Disable(SPI_MLINE_SPI);
    return 0;
}
static int open(struct mliner_dev * dev, const struct mliner_dev_params * params)
{
    dev->is_opened = 1;
    TxLen = params->len;
    RxLen = params->len;
    irq_attach(SPI_MLINE_DMA_IRQ_TX, TransmitIRQhandler, 0, NULL, "TransmitIRQhandler");
    irq_attach(SPI_MLINE_DMA_IRQ_RX, ReceiveIRQhandler, 0, NULL, "ReceiveIRQhandler");
    LL_DMA_ConfigAddresses(SPI_MLINE_DMA, 
                            SPI_MLINE_DMA_STREAM_RX,
                           LL_SPI_DMA_GetRegAddr(SPI_MLINE_SPI),
							(uint32_t)RxBuffer,
                           LL_DMA_GetDataTransferDirection(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX));
    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, RxLen );

    LL_DMA_ConfigAddresses(SPI_MLINE_DMA,
                           SPI_MLINE_DMA_STREAM_TX, 
							(uint32_t)TxBuffer,
                           LL_SPI_DMA_GetRegAddr(SPI_MLINE_SPI),
                           LL_DMA_GetDataTransferDirection(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX));
    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, TxLen );


    LL_DMA_EnableIT_TC(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);
    LL_DMA_EnableIT_TE(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);
    LL_DMA_EnableIT_TC(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);
    LL_DMA_EnableIT_TE(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);

    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, TxLen); //устанавливаем сколько символов передачть
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, RxLen); //устанавливаем сколько символов передачть
    LL_SPI_EnableDMAReq_RX(SPI_MLINE_SPI);
    LL_SPI_EnableDMAReq_TX(SPI_MLINE_SPI);
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    LL_SPI_Enable(SPI_MLINE_SPI);

    return 0;
}

EMBOX_UNIT_INIT(initSpiMlinerMain);
static int initSpiMlinerMain(void)
{
    mlimpl_initBoard();
   for(int i = 0; i < MLINER_BASEDEV_DEVSBLOCK_CNT; i++)
   {
      if(MlinerDevsBlock.devices[i].is_enabled == 0)
      {
         MlinerDev = &MlinerDevsBlock.devices[i];
         MlinerDev->is_enabled = 1;
		 MlinerDev->driver = &SpiMainDev;
         MlinerDev->driver->open = open;
         MlinerDev->driver->close = close;
         MlinerDev->driver->read = read;
         MlinerDev->driver->updateRx = updateRx;
         MlinerDev->driver->updateTxRx = updateTxRx;
		 struct mliner_dev_params params;
		 params.len = MLINE_RXTX_BUFFER_SIZE;
		 open(MlinerDev, &params);

         break;
      }
   }

    return 0;
}


int mlimpl_initBoard(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    //LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    SPI_MLINE_ENABLE_CLOCK_SPI

    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    SPI_MLINE_ENABLE_GPIO
    /**SPI1 GPIO Configuration
     PA5   ------> SPI1_SCK
    PA7   ------> SPI1_MOSI
    PB4   ------> SPI1_MISO
    */
    GPIO_InitStruct.Pin = SPI_MLINE_SCK_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_SCK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_MLINE_MOSI_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_MOSI_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_MLINE_MISO_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_MISO_PORT, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_RX Init */
    LL_DMA_SetChannelSelection      (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, SPI_MLINE_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel   (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode          (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode  (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);

      /* SPI1_TX Init */
    LL_DMA_SetChannelSelection      (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, SPI_MLINE_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel   (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode          (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode  (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);

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
    LL_SPI_Init(SPI_MLINE_SPI, &SPI_InitStruct);

    return 0;
}

