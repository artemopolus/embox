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
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx.h"

#include "mliner/mliner_secdev_impl.h"


int mlimpl_initBoard(void)
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

   return 0;
}
void mlimpl_setBoardBuffer(
	spi_mline_dev_t * transmit, spi_mline_dev_t * receive
	// uint8_t * rxdata, uint32_t rxlen, uint8_t * txdata, uint32_t txlen, int (*download)(uint32_t len), int(*upload)(uint32_t len)
	)
{
	LL_DMA_ConfigAddresses(DMA1,
									LL_DMA_CHANNEL_4,
									LL_SPI_DMA_GetRegAddr(SPI2),
									(uint32_t)receive->dmabufferdata,
									LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, receive->dmabufferlen);
	LL_DMA_ConfigAddresses(DMA1,
									LL_DMA_CHANNEL_5, 
									(uint32_t)transmit->dmabufferdata,
									LL_SPI_DMA_GetRegAddr(SPI2),
									LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, transmit->dmabufferlen);
}
uint8_t mlimpl_runBoardIRQhandlerRX(void)
{
	uint8_t res = 0;

 	if (LL_DMA_IsActiveFlag_TC4(DMA1) != RESET)
	{
		LL_DMA_ClearFlag_GI4(DMA1);
	//   lthread_launch(&SPI2_FULL_DMA_rx_buffer.dt_lth);
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
		res = 1;
	}

	return res;
}
uint8_t mlimpl_runBoardIRQhandlerTX(void)
{
	uint8_t res = 0;
	if (LL_DMA_IsActiveFlag_TC5(DMA1) != RESET)
	{
		LL_DMA_ClearFlag_GI5(DMA1);
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
	//   lthread_launch(&SPI2_FULL_DMA_tx_buffer.dt_lth);
		res = 1;
	}
	return res;
}
void mlimpl_enableBoard(uint32_t rxlen, uint32_t txlen)
{
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_5);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, txlen);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, rxlen);

	LL_SPI_EnableDMAReq_RX(SPI2);
	LL_SPI_EnableDMAReq_TX(SPI2);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);   //transmit
	LL_SPI_Enable(SPI2);
}
void mlimpl_disableBoard(void)
{
	LL_SPI_Disable(SPI2);
	LL_SPI_DeInit(SPI2);
	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_5);
}

void mlimpl_receiveBoard(spi_mline_dev_t * receiver)
{
	    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
   //  for (uint16_t i = 0; i < _datacount; i++)
   //  {
   //      pshfrc_exbu8(&_trg_thread->datastorage, SPI2_FULL_DMA_rx_buffer.dt_buffer[i]);
   //      SPI2_FULL_DMA_rx_buffer.dt_buffer[i] = 0;
   //  }
   //  SPI2_FULL_DMA_rx_buffer.is_full = 0;
   //  _trg_thread->isready = 0;
	// receiver->processData(receiver->dmabufferlen);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, receiver->dmabufferlen);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
}

void mlimpl_resetBoardRx(spi_mline_dev_t * receiver)
{
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, receiver->dmabufferlen);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
}
void mlimpl_resetBoardRxTx(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit)
{
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, transmit->dmabufferlen);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, receiver->dmabufferlen);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
}
void mlimpl_receiveTransmitBoard(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit)
{
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
   LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit

	// receiver->processData(receiver->dmabufferlen);
	transmit->processData(transmit->dmabufferlen);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, transmit->dmabufferlen);
   LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, receiver->dmabufferlen);
   LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
   LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
}

