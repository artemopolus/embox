#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_exti.h"
#include <embox/unit.h>
#include <errno.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "gpio/gpio.h"

static struct lthread   Ex_Gpio_Lthread;
uint8_t                 Ex_Gpio_IsEnabled = 0;

static irq_return_t gpio_spi_irq_handler(unsigned int irq_nr, void *data)
{
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_12) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12);

    /* Manage code in main.c.*/
    // UserButton_Callback(); 
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(8, gpio_spi_irq_handler, NULL);

EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_12, LL_GPIO_MODE_INPUT );
    //   EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt 
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_12;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    irq_attach(8, gpio_spi_irq_handler, 0, NULL, "gpio spi irq handler");
    return 0;
}

void ex_enableGpio()
{
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
}
void ex_disableGpio()
{
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12);
}
uint32_t ex_checkGpio()
{
    uint32_t result = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_12);
    return result;
}
uint8_t ex_subscribeOnGpioEvent( exacto_gpio_types_t type ,int (*run)(struct lthread *))
{
    if (Ex_Gpio_IsEnabled)
        return 1;
    lthread_init(&Ex_Gpio_Lthread, run);
    Ex_Gpio_IsEnabled = 1;
    return 0;
}
void ex_setOutputGpio(exacto_gpio_types_t type)
{
    // LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_12);
    // LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_12);
    LL_EXTI_DeInit();
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT );
}
