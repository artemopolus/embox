#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include <errno.h>
#include <embox/unit.h>

#define LIGHT_BLUE_Pin LL_GPIO_PIN_7
#define LIGHT_BLUE_GPIO_Port GPIOE
#define LIGHT_GREEN_Pin LL_GPIO_PIN_8
#define LIGHT_GREEN_GPIO_Port GPIOE


EMBOX_UNIT_INIT(initAfinaBlinkBasis);
static int initAfinaBlinkBasis(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

    LL_GPIO_ResetOutputPin(LIGHT_BLUE_GPIO_Port, LIGHT_BLUE_Pin);
    LL_GPIO_ResetOutputPin(LIGHT_GREEN_GPIO_Port, LIGHT_GREEN_Pin);

    GPIO_InitStruct.Pin = LIGHT_BLUE_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(LIGHT_BLUE_GPIO_Port, &GPIO_InitStruct);

  /**/
    GPIO_InitStruct.Pin = LIGHT_GREEN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(LIGHT_GREEN_GPIO_Port, &GPIO_InitStruct);

    return 0;
}
void setSysLedOn()
{
    LL_GPIO_SetOutputPin(LIGHT_GREEN_GPIO_Port, LIGHT_GREEN_Pin);
}
void setSysLedOff()
{
    LL_GPIO_ResetOutputPin(LIGHT_GREEN_GPIO_Port, LIGHT_GREEN_Pin);

}
void setDataLedOn()
{

}
void setDataLedOff()
{

}
