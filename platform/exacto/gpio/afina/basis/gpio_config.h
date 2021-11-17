#ifndef GPIO_CONFIG_H
#define  GPIO_CONFIG_H

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

#define GPIO_AFINA_BASIS_CONFIG_ENABLED

#define SPI_MLINE_SPI SPI1

#define SPI_MLINE_ENABLE_CLOCK_SPI LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

#define SPI_MLINE_ENABLE_GPIO LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA); \
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

#define SPI_MLINE_SCK_PIN LL_GPIO_PIN_5
#define SPI_MLINE_SCK_PORT GPIOA
#define SPI_MLINE_MOSI_PIN LL_GPIO_PIN_7
#define SPI_MLINE_MOSI_PORT GPIOA
#define SPI_MLINE_MISO_PIN LL_GPIO_PIN_4
#define SPI_MLINE_MISO_PORT GPIOB

#define SPI_MLINE_CS_PIN LL_GPIO_PIN_12
#define SPI_MLINE_CS_PORT GPIOE
#define SPI_MLINE_ENABLE_CLOCK_CS LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

#define SPI_MLINE_DMA DMA2
#define SPI_MLINE_DMA_STREAM_TX	LL_DMA_STREAM_5
#define SPI_MLINE_DMA_STREAM_RX	LL_DMA_STREAM_0
#define SPI_MLINE_DMA_CHANNEL LL_DMA_CHANNEL_3

#define SPI_MLINE_DMA_IRQ_RX 56
#define SPI_MLINE_DMA_IRQ_TX 68


#define LED_RED_PIN
#define LED_RED_PORT 
//#define LED_RED_ON 

#define LED_GREEN_PIN LL_GPIO_PIN_8
#define LED_GREEN_PORT GPIOE
#define LED_GREEN_ON

#define LED_BLUE_PIN LL_GPIO_PIN_7
#define LED_BLUE_PORT GPIOE
#define LED_BLUE_ON 


#define LED_ENABLE_CLOCK LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

#endif
