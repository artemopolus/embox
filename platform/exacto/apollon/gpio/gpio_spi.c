#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>

EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_12, LL_GPIO_MODE_INPUT );
    return 0;
}

void ex_enableGpio()
{

}
void ex_disableGpio()
{

}
uint32_t ex_checkGpio()
{
    uint32_t result = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_12);
    return result;
}
