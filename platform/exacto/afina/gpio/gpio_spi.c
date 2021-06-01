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
void ex_toggleLed(ex_led_type_t type)
{
    switch (type)
    {
    case EX_LED_RED:
        break;
    case EX_LED_GREEN:
        LL_GPIO_TogglePin(GPIOE, LL_GPIO_PIN_8);
        break;
    case EX_LED_BLUE:
        LL_GPIO_TogglePin(GPIOE, LL_GPIO_PIN_7);
        break;
    case EX_LED_RGB:
        break;
    default:
        break;
    }

}
void ex_enableGpio(exacto_gpio_types_t type)
{
    switch (type)
    {
    case EX_GPIO_SPI_MLINE:
        LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_12);
        break;
    case EX_GPIO_SPI_SYNC:
        LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_15);
        break;
    default:
        break;
    }
}
void ex_disableGpio(exacto_gpio_types_t type)
{
    switch (type)
    {
    case EX_GPIO_SPI_MLINE:
        LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_12);
        break;
    case EX_GPIO_SPI_SYNC:
        LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_15);
        break;
    default:
        break;
    }

}
uint32_t ex_checkGpio(exacto_gpio_types_t type)
{
    uint32_t result = LL_GPIO_IsInputPinSet(GPIOE, LL_GPIO_PIN_12);
    return result;
}

EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

    //i2c4 sda using pf15
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
    LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_15, LL_GPIO_MODE_OUTPUT );

    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT );
    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT );
    LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT );
    ex_enableGpio(EX_GPIO_SPI_MLINE);
    ex_enableGpio(EX_GPIO_SPI_SYNC);
    ex_disableLed(EX_LED_GREEN);
    ex_disableLed(EX_LED_BLUE);
    return 0;
}
// uint8_t ex_subscribeOnGpioEvent( exacto_gpio_types_t type ,int (*run)(struct lthread *))
// {
//     return 0;
// }
void ex_setOutputGpio(exacto_gpio_types_t type)
{

}

