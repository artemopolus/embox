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

#include "mliner/mliner_maindev_impl.h"

#include "gpio/gpio.h"

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
struct mliner_dev_ops SpiSecDev;


static irq_return_t TransmitIRQhandler(unsigned int irq_nr, void *data)
{
	if (LL_DMA_IsActiveFlag_TC5(DMA1) != RESET)
	{
		LL_DMA_ClearFlag_GI5(DMA1);
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
		MlinerDev->readyTx = 1;
	}
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(15, TransmitIRQhandler, NULL);
static irq_return_t ReceiveIRQhandler(unsigned int irq_nr, void *data)
{
 	if (LL_DMA_IsActiveFlag_TC4(DMA1) != RESET)
	{
		LL_DMA_ClearFlag_GI4(DMA1);
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
		MlinerDev->readyRx = 1;
	} 
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(14, ReceiveIRQhandler, NULL);




static int updateRx(struct mliner_dev * dev)
{
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
    	dev->receiveDataProcess(RxBuffer, RxLen);
		LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, RxLen);
		LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
	}
	return 0;
}
static int updateTxRx(struct mliner_dev * dev)
{
	if(!dev->readyRx)
	{
		dev->driver->updateRx(dev);
		return 0;
	}
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		if(dev->readyTx)
		{
			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit

			dev->receiveDataProcess(RxBuffer, RxLen);
			dev->transmitDataProcess(TxBuffer, TxLen);

			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, TxLen);
			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, RxLen);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
			dev->readyRx = 0;
			dev->readyTx = 0;
			return 1;
		}
		else
		{
			dev->driver->updateRx(dev);
		}
	}
	return 0;
}
static int read(struct mliner_dev * dev, uint8_t * data, uint16_t len)
{
	return 0;
}
static int close (struct mliner_dev * dev)
{
	dev->is_opened = 0;
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_5); //transmit
	irq_detach(15,NULL);
	irq_detach(14,NULL);
	LL_SPI_Disable(SPI2);
	LL_SPI_DeInit(SPI2);
	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_DeInit(DMA1, LL_DMA_CHANNEL_5);
	return 0;
}
static int open(struct mliner_dev * dev, const struct mliner_dev_params * params)
{
	dev->is_opened = 1;
	dev->params = *params;
	dev->readyRx = 0;
	dev->readyTx = 1;
	TxLen = params->len;
	RxLen = params->len;
	mlimpl_initBoard();
   irq_attach(15, TransmitIRQhandler, 0, NULL, "TransmitIRQhandler");
   irq_attach(14, ReceiveIRQhandler, 0, NULL, "ReceiveIRQhandler");

	LL_DMA_ConfigAddresses(DMA1,
									LL_DMA_CHANNEL_4,
									LL_SPI_DMA_GetRegAddr(SPI2),
									(uint32_t)RxBuffer,
									LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, RxLen);
	LL_DMA_ConfigAddresses(DMA1,
									LL_DMA_CHANNEL_5, 
									(uint32_t)TxBuffer,
									LL_SPI_DMA_GetRegAddr(SPI2),
									LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5));
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, TxLen);


	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_4);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_5);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_5);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, TxLen);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, RxLen);

	LL_SPI_EnableDMAReq_RX(SPI2);
	LL_SPI_EnableDMAReq_TX(SPI2);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);   //transmit
	LL_SPI_Enable(SPI2);
	return 0;
}



EMBOX_UNIT_INIT(initSpiMlinerSec);
static int initSpiMlinerSec(void)
{
	for(int i = 0; i < MLINER_BASEDEV_DEVSBLOCK_CNT; i++)
	{
		if(MlinerDevsBlock.devices[i].is_enabled == 0)
		{
			MlinerDev = &MlinerDevsBlock.devices[i];
			MlinerDev->is_enabled = 1;
			MlinerDev->driver = &SpiSecDev;
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
