#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
typedef enum e_l_t_t{
    EX_LED_RED = 0,
    EX_LED_GREEN,
    EX_LED_BLUE,
    EX_LED_RGB
}ex_led_type_t;
typedef enum{
    EX_GPIO_SPI_MLINE,
    EX_GPIO_SPI_SNS,
    EX_GPIO_SPI_SYNC
}exacto_gpio_types_t;
extern void ex_enableLed(ex_led_type_t type);
extern void ex_disableLed(ex_led_type_t type);
extern void ex_toggleLed(ex_led_type_t type);
extern void ex_enableGpio(exacto_gpio_types_t type);
extern void ex_disableGpio(exacto_gpio_types_t type);
extern uint32_t ex_checkGpio(exacto_gpio_types_t type);
// extern uint8_t ex_subscribeOnGpioEvent( exacto_gpio_types_t type ,int (*run)(struct lthread *));
extern void ex_setOutputGpio(exacto_gpio_types_t type);
#endif //SPI2_GENERATED_H

