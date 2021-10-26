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


#include "i2c_mliner.h"


#include "commander/exacto_data_storage.h"



typedef struct
{
    struct lthread dt_lth;
    uint8_t dt_buffer[BUFFER_SIZE_I2C_MLINER];
    uint8_t is_enabled;
    uint16_t dt_count;
} buffer;

static buffer RX_buffer = {
	.is_enabled = 0,
	.dt_count = BUFFER_SIZE_I2C_MLINER,
};
static buffer TX_buffer = {
	.is_enabled = 0,
	.dt_count = BUFFER_SIZE_I2C_MLINER,
};


static uint8_t IsEnabled = 0;
static irq_return_t RX_DmaHandler(unsigned int irq_nr, void *data);
static irq_return_t TX_DmaHandler(unsigned int irq_nr, void *data);

EMBOX_UNIT_INIT(init_I2C_MLINER);
static int init_I2C_MLINER(void)
{
	/* general cubemx section */
  LL_I2C_InitTypeDef I2C_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    /* I2C1_RX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_1);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_0);

  /* I2C1_TX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_1);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_6);

	LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x20404768;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 154;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);

	/* DMA config */
    // DMA1_STREAM_0 -> RX
    LL_DMA_ConfigAddresses(DMA1, 
                            LL_DMA_STREAM_0,
                           LL_I2C_DMA_GetRegAddr(I2C1, LL_I2C_DMA_REG_DATA_RECEIVE), (uint32_t)RX_buffer.dt_buffer,
                           LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_STREAM_0));
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, RX_buffer.dt_count);    
    // DMA1_STREAM_6 -> TX
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_6,
			(uint32_t)TX_buffer.dt_buffer, (uint32_t)LL_I2C_DMA_GetRegAddr(I2C1, LL_I2C_DMA_REG_DATA_TRANSMIT),
			LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_STREAM_6));



    /* DMA interrupts */
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_0); 
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_6);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_6); 

    /* embox specific section  */
    irq_attach(11, RX_DmaHandler, 0, NULL, "I2C1 rx DMA handler");
    irq_attach(17, TX_DmaHandler, 0, NULL, "I2C1 rx DMA handler");
    

	return 0;
}
void enable_I2C_MLINER(void)
{
  if(!IsEnabled)
  { 
    /* enable i2c */
    LL_I2C_EnableDMAReq_RX(I2C1);
    LL_I2C_EnableDMAReq_TX(I2C1);
    LL_I2C_Enable(I2C1);
    IsEnabled = 1;
  }

}

static irq_return_t RX_DmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC0(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_TC0(DMA1);
	if (RX_buffer.is_enabled)
		lthread_launch(&RX_buffer.dt_lth);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(11, RX_DmaHandler, NULL); 
//  DMA1_Stream0_IRQn           = 11,     /*!< DMA1 Stream 0 global Interrupt
static irq_return_t TX_DmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC6(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_TC6(DMA1);
	if (TX_buffer.is_enabled)
		lthread_launch(&TX_buffer.dt_lth);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(17, TX_DmaHandler, NULL); 
//DMA1_Stream6_IRQn           = 17,     /*!< DMA1 Stream 6 global Interrupt   
void bind_TX_thread_I2C_MLINER(int (*run)(struct lthread *))
{
	if (TX_buffer.is_enabled == 0)
	{
		TX_buffer.is_enabled = 1;
		lthread_init(&TX_buffer.dt_lth, run);
	}
}
void bind_RX_thread_I2C_MLINER(int (*run)(struct lthread *))
{

}
void transmit_I2C_MLINER(uint8_t * data, const uint16_t datalen, uint32_t address)
{
  /* (1) Enable DMA transfer **************************************************/
  LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
  /* (2) Initiate a Start condition to the Slave device ***********************/

  /* Master Generate Start condition for a write request:
   *  - to the Slave with a 7-Bit SLAVE_OWN_ADDRESS
   *  - with a auto stop condition generation when transmit all bytes
   */
  LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, BUFFER_SIZE_I2C_MLINER, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
}
void receive_I2C_MLINER(uint8_t * data, const uint16_t datalen)
{
	
}
