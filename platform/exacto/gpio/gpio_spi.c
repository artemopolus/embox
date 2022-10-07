#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "gpio/gpio_spi.h"

#include "gpio_config.h"

#ifndef GPIO_AFINA_BASIS_CONFIG_ENABLED
#error Unsupported platform
#endif

void ex_toggleGpio(exacto_gpio_types_t type)
{
    switch (type)
    {
    case EX_GPIO_SPI_MLINE:
        LL_GPIO_TogglePin(SPI_MLINE_CS_PORT, SPI_MLINE_CS_PIN );
        break;
    case EX_GPIO_SPI_SYNC:
        LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_15);
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
        LL_GPIO_SetOutputPin(SPI_MLINE_CS_PORT, SPI_MLINE_CS_PIN );
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
        LL_GPIO_ResetOutputPin(SPI_MLINE_CS_PORT, SPI_MLINE_CS_PIN);
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
    uint32_t result = LL_GPIO_IsInputPinSet(SPI_MLINE_CS_PORT, SPI_MLINE_CS_PIN);
    return result;
}

EMBOX_UNIT_INIT(initSpiGpio);
static int initSpiGpio(void)
{
    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    SPI_MLINE_ENABLE_CLOCK_CS

    LL_GPIO_SetPinMode(SPI_MLINE_CS_PORT, SPI_MLINE_CS_PIN, LL_GPIO_MODE_OUTPUT );

    ex_enableGpio(EX_GPIO_SPI_MLINE);
    return 0;
}

void ex_setOutputGpio(exacto_gpio_types_t type)
{

}

