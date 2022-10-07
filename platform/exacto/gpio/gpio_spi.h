#ifndef EXACTO_GPIO_GPIO_SPI_H
#define EXACTO_GPIO_GPIO_SPI_H
#include <stdint.h>
typedef enum{
    EX_GPIO_SPI_MLINE,
    EX_GPIO_SPI_SNS,
    EX_GPIO_SPI_SYNC
}exacto_gpio_types_t;
extern void ex_enableGpio(exacto_gpio_types_t type);
extern void ex_disableGpio(exacto_gpio_types_t type);
extern void ex_toggleGpio(exacto_gpio_types_t type);
extern uint32_t ex_checkGpio(exacto_gpio_types_t type);
extern void ex_setOutputGpio(exacto_gpio_types_t type);

#endif
