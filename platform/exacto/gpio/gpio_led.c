#include "gpio/gpio_led.h"
#include "gpio_config.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>

void ex_enableLed(ex_led_type_t type)
{
    switch (type)
    {
    case EX_LED_RED:
        break;
    case EX_LED_GREEN:
#ifdef LED_GREEN_ON
        LL_GPIO_SetOutputPin( LED_GREEN_PORT, LED_GREEN_PIN );
#endif
        break;
    case EX_LED_BLUE:
#ifdef LED_BLUE_ON
        LL_GPIO_SetOutputPin( LED_BLUE_PORT, LED_BLUE_PIN );
#endif
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
#ifdef LED_GREEN_ON
        LL_GPIO_ResetOutputPin(LED_GREEN_PORT, LED_GREEN_PIN);
#endif
        break;
    case EX_LED_BLUE:
#ifdef LED_BLUE_ON
        LL_GPIO_ResetOutputPin( LED_BLUE_PORT, LED_BLUE_PIN);
#endif
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
#ifdef LED_GREEN_ON
        LL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);
#endif
        break;
    case EX_LED_BLUE:
#ifdef LED_BLUE_ON
        LL_GPIO_TogglePin( LED_BLUE_PORT, LED_BLUE_PIN);
#endif
        break;
    case EX_LED_RGB:
        break;
    default:
        break;
    }

}


EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

    LED_ENABLE_CLOCK


#ifdef LED_GREEN_ON
    LL_GPIO_SetPinMode(LED_GREEN_PORT, LED_GREEN_PIN, LL_GPIO_MODE_OUTPUT );
#endif
#ifdef LED_BLUE_ON
    LL_GPIO_SetPinMode( LED_BLUE_PORT, LED_BLUE_PIN, LL_GPIO_MODE_OUTPUT );
#endif
#ifdef LED_GREEN_ON
    ex_disableLed(EX_LED_GREEN);
#endif
#ifdef LED_BLUE_ON
    ex_disableLed(EX_LED_BLUE);
#endif
    return 0;
}
