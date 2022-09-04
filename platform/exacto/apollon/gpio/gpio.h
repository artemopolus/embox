#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
#include <errno.h>
#include <kernel/lthread/lthread.h>
// #include "commander/exacto_data_storage.h"

typedef enum{
    EX_GPIO_SPI_MLINE,
    EX_GPIO_SPI_SNS,
    EX_GPIO_SPI_SYNC
}exacto_gpio_types_t;
extern void ex_enableGpio(exacto_gpio_types_t type);
extern void ex_disableGpio(exacto_gpio_types_t type);
extern uint32_t ex_checkGpio(exacto_gpio_types_t type);

extern uint8_t ex_subscribeOnGpioEvent( exacto_gpio_types_t type ,int (*run)(struct lthread *));
extern void ex_setOutputGpio(exacto_gpio_types_t type);
#endif //SPI2_GENERATED_H

