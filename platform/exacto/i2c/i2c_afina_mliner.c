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


#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/printk.h>
#include "commander/exacto_data_storage.h"


typedef struct
{
    struct lthread dt_lth;
    uint8_t is_enabled;
} buffer;

static buffer TX_buffer = {
	.is_enabled = 0,
};

EMBOX_UNIT_INIT(init_I2C_MLINER);
static int init_I2C_MLINER(void)
{
	return 0;
}

static irq_return_t TX_DmaHandler(unsigned int irq_nr, void *data)
{
    if (LL_DMA_IsActiveFlag_TC0(DMA1) != RESET)
    {
        LL_DMA_ClearFlag_TC0(DMA1);
	lthread_launch(&TX_buffer.dt_lth);
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(11, TX_DmaHandler, NULL); 
//  DMA1_Stream0_IRQn           = 11,     /*!< DMA1 Stream 0 global Interrupt

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

