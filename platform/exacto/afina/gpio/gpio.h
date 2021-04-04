#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>
typedef enum e_l_t_t{
    EX_LED_RED = 0,
    EX_LED_GREEN,
    EX_LED_BLUE,
    EX_LED_RGB
}ex_led_type_t;
extern void ex_enableLed(ex_led_type_t type);
extern void ex_disableLed(ex_led_type_t type);
extern void ex_enableGpio();
extern void ex_disableGpio();
extern uint32_t ex_checkGpio();
#endif //SPI2_GENERATED_H

