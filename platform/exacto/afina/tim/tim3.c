#include "stm32f7xx_hal.h"
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include "commander/exacto_data_storage.h"

#include "tim.h"
ex_subs_service_t ExTimServices[TIM_SERVICES_COUNT];
ex_service_info_t ExTimServicesInfo = {
  .max_count = TIM_SERVICES_COUNT,
  .current_count = 0,
};

#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()


/* Definition for TIMx's NVIC */
#define TIMx_IRQn                      TIM3_IRQn
#define TIMx_IRQHandler                TIM3_IRQHandler


TIM_HandleTypeDef    TimHandle;

/* Prescaler declaration */
uint32_t uwPrescalerValue = 0;
static irq_return_t runAfinaTim3IrqHandler(unsigned int irq_nr, void *data)
{
    HAL_TIM_IRQHandler(&TimHandle);
// startTickReactionThread( );
   ex_updateEventForSubs(ExTimServicesInfo, ExTimServices, THR_TIM); 
    
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(29, runAfinaTim3IrqHandler, NULL);

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripheral clock #################################*/
  /* TIMx Peripheral clock enable */
  TIMx_CLK_ENABLE();
  irq_attach(29, runAfinaTim3IrqHandler, 0, NULL,
   "runAfinaTim3IrqHandler"); 
  
  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
//   HAL_NVIC_SetPriority(TIMx_IRQn, 3, 0);

  /* Enable the TIMx global Interrupt */
//   HAL_NVIC_EnableIRQ(TIMx_IRQn);
}


EMBOX_UNIT_INIT(initAfinaTim3);

static int initAfinaTim3(void)
{
uwPrescalerValue = (uint32_t)((SystemCoreClock / 2) / 10000) - 1;

  /* Set TIMx instance */
  TimHandle.Instance = TIMx;

  /* Initialize TIMx peripheral as follows:
       + Period = 10000 - 1
       + Prescaler = ((SystemCoreClock / 2)/10000) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
  TimHandle.Init.Period            = 1000 - 1;
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    // Error_Handler();
  }

  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {
    /* Starting Error */
    // Error_Handler();
  }
  
  ex_initSubscribeEvents(ExTimServicesInfo, ExTimServices);  
  return 0;
}
void ex_setFreqHz(const uint32_t target_freq)
{
  uint32_t value = 100;
  switch (target_freq)
  {
    case 100:
      value = 100;
      break;
    case 200:
      value = 50;
      break;
    default:
      return;
      break;
  }
  HAL_TIM_Base_Stop_IT(&TimHandle);
  TimHandle.Init.Period = value - 1;
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {}
  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {}
}
