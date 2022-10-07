#ifndef EXACTO_GPIO_GPIO_LED_H
#define EXACTO_GPIO_GPIO_LED_H
#include <stdint.h>
typedef enum e_l_t_t{
    EX_LED_RED = 0,
    EX_LED_GREEN,
    EX_LED_BLUE,
    EX_LED_RGB
}ex_led_type_t;
extern void ex_enableLed(ex_led_type_t type);
extern void ex_disableLed(ex_led_type_t type);
extern void ex_toggleLed(ex_led_type_t type);

#endif
