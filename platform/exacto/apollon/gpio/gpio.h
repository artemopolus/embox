#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
#include <errno.h>
#include <kernel/lthread/lthread.h>
typedef enum{
    EX_GPIO_SPI_MLINE,
    EX_GPIO_SPI_SNS
}exacto_gpio_types_t;

extern void ex_enableGpio();
extern void ex_disableGpio();
extern uint32_t ex_checkGpio();
extern uint8_t ex_subscribeOnGpioEvent( exacto_gpio_types_t type ,int (*run)(struct lthread *));
extern void ex_setOutputGpio(exacto_gpio_types_t type);
#endif //SPI2_GENERATED_H

