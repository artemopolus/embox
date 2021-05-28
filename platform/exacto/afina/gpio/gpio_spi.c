#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "gpio/gpio.h"
void ex_enableLed(ex_led_type_t type)
{
    switch (type)
    {
    case EX_LED_RED:
        break;
    case EX_LED_GREEN:
        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_8);
        break;
    case EX_LED_BLUE:
        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_7);
        break;
    case EX_LED_RGB:
        break;
    default:
        break;
    }
}
void ex_disableLed(ex_led_type_t type)
{
    switch (type)
    {
    case EX_LED_RED:
        break;
    case EX_LED_GREEN:
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_8);
        break;
    case EX_LED_BLUE:
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_7);
        break;
    case EX_LED_RGB:
        break;
    default:
        break;
    }
}
void ex_enableGpio()
{
    LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_12);
}
void ex_disableGpio()
{
    LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_12);

}
uint32_t ex_checkGpio()
{
    uint32_t result = LL_GPIO_IsInputPinSet(GPIOE, LL_GPIO_PIN_12);
    return result;
}

EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT );
    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT );
    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT );
    ex_enableGpio();
    ex_disableLed(EX_LED_GREEN);
    ex_disableLed(EX_LED_BLUE);
    return 0;
}

