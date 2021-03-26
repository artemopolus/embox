#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_tim.h"
#include "system_stm32f1xx.h"
#include "stm32f1xx.h"
#include <kernel/lthread/lthread.h>

#include <embox/unit.h>
#include <kernel/irq.h>

#include "tim.h"
ex_subs_service_t ExTimServices[TIM_SERVICES_COUNT];
ex_service_info_t ExTimServicesInfo = {
  .max_count = TIM_SERVICES_COUNT,
  .current_count = 0,
};

/* Initial autoreload value */

/* Actual autoreload value multiplication factor */
// static uint8_t AutoreloadMult = 1;
// static struct lthread tim_irq_lt;

/* TIM2 Clock */

static irq_return_t tim_irq_handler(unsigned int irq_nr, void *data);
// static int tim_handler(struct lthread *self);

EMBOX_UNIT_INIT(apollon_tim_init);
static int apollon_tim_init(void)
{
  static uint32_t TimOutClock = 1;

  static uint32_t InitialAutoreload = 0;
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  LL_TIM_SetPrescaler(TIM3, __LL_TIM_CALC_PSC(SystemCoreClock, 10000));
  
  /* Set the auto-reload value to have an initial update event frequency of 10 Hz */
    /* TIM2CLK = SystemCoreClock / (APB prescaler & multiplier)                 */
  TimOutClock = SystemCoreClock/2;
  
  InitialAutoreload = __LL_TIM_CALC_ARR(TimOutClock, LL_TIM_GetPrescaler(TIM3), 10);
  LL_TIM_SetAutoReload(TIM3, InitialAutoreload);
  // LL_TIM_InitTypeDef TIM_InitStruct = {0};

  // TimOutClock = SystemCoreClock / 2;
  // InitialAutoreload = __LL_TIM_CALC_ARR(TimOutClock, LL_TIM_GetPrescaler(TIM3), 10);

  // TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 10000);
  // TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  // TIM_InitStruct.Autoreload = InitialAutoreload;
  // TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;

  // LL_TIM_Init(TIM3, &TIM_InitStruct);


  uint8_t res = 0;

  res |= irq_attach(29, tim_irq_handler, 0, NULL, "tim_irq_handler");
  
  // lthread_init(&tim_irq_lt, &tim_handler);
  // schedee_priority_set(&tim_irq_lt.schedee, 200);
  
  LL_TIM_EnableIT_UPDATE(TIM3);
  LL_TIM_EnableCounter(TIM3);
  LL_TIM_GenerateEvent_UPDATE(TIM3);


  ex_initSubscribeEvents(ExTimServicesInfo, ExTimServices);  

  return 0;
}

// static int tim_handler(struct lthread *self)
// {
//   // timer_strat_sched(cs_jiffies->jiffies);
//   return 0;
// }
static irq_return_t tim_irq_handler(unsigned int irq_nr, void *data)
{
  /* Check whether update interrupt is pending */
  if (LL_TIM_IsActiveFlag_UPDATE(TIM3) == 1)
  {
    /* Clear the update interrupt flag*/
    LL_TIM_ClearFlag_UPDATE(TIM3);
    ex_updateEventForSubs(ExTimServicesInfo, ExTimServices, THR_TIM); 
  }
  /* lthread gogogogo */
  // lthread_launch(&tim_irq_lt);

  return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(29, tim_irq_handler, NULL);
