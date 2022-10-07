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
#include "kernel/printk.h"

#include "i2c_master.h"


EMBOX_UNIT_INIT(initI2c1Master);
static int initI2c1Master(void)
{
	LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.ClockSpeed = 100000;
  I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0);

  LL_I2C_Enable(I2C1);
	return 0;
}

uint8_t ex_send_i2c_m(ex_i2c_pack_t *  input)
{
	return 0;
}

uint8_t __attribute__((optimize("O0")))ex_gett_i2c_m(ex_i2c_pack_t * output)
{
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);

	LL_I2C_GenerateStartCondition(I2C1);

	I2C_APPOLON_INDEX_SZ_INT index = 0;
	while(!LL_I2C_IsActiveFlag_SB(I2C1))
	{
		if(index++ > I2C_APPOLON_INDEX_MAX)
			return 1;
	}

	LL_I2C_TransmitData8(I2C1, output->address | I2C_REQUEST_WRITE);

	index = 0;
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
	{
		if(index++ > I2C_APPOLON_INDEX_MAX)
			return 1;
	}
	LL_I2C_ClearFlag_ADDR(I2C1);

	uint8_t ubNbDataToTransmit = output->datalen;

	index = 0;

	uint8_t pt = 0;

	while(ubNbDataToTransmit > 0)
	{
		if(LL_I2C_IsActiveFlag_TXE(I2C1))
		{
			LL_I2C_TransmitData8(I2C1, output->data[pt++]);
			ubNbDataToTransmit--;
		}
		if(index++ > I2C_APPOLON_INDEX_MAX)
			return 1;
	}
	LL_I2C_GenerateStopCondition(I2C1);

	return 0;
}
