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
static exacto_tim_states_t ExTim_Freq_Current;

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
  TimOutClock = SystemCoreClock/1;
  
  InitialAutoreload = __LL_TIM_CALC_ARR(TimOutClock, LL_TIM_GetPrescaler(TIM3), 100);
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
    ex_updateEventForSubs(ExTimServicesInfo, ExTimServices, EX_THR_TIM); 
    // EDS_spidmairq_Marker = 1;
  }
  /* lthread gogogogo */
  // lthread_launch(&tim_irq_lt);

  return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(29, tim_irq_handler, NULL);
void ex_setFreqHz(const uint32_t target_freq)
{
  uint32_t value = 100;
  switch (target_freq)
  {
    case 1:
      value = 1;
      ExTim_Freq_Current = EXACTO_TIM_1;
      break;
    case 10:
      value = 10;
      ExTim_Freq_Current = EXACTO_TIM_10;
      break;
    case 50:
      value = 50;
      ExTim_Freq_Current = EXACTO_TIM_50;
      break;
    case 100:
      value = 100;
      ExTim_Freq_Current = EXACTO_TIM_100;
      break;
    case 200:
      value = 200;
      ExTim_Freq_Current = EXACTO_TIM_200;
      break;
    case 400:
      value = 400;
      ExTim_Freq_Current = EXACTO_TIM_400;
      break;
    case 800:
      value = 800;
      ExTim_Freq_Current = EXACTO_TIM_800;
      break;
    case 1000:
      value = 1000;
      ExTim_Freq_Current = EXACTO_TIM_1000;
      break;
    case 1600:
      value = 1600;
      ExTim_Freq_Current = EXACTO_TIM_1600;
      break;
    case 2000:
      value = 2000;
      ExTim_Freq_Current = EXACTO_TIM_2000;
      break;
    case 3200:
      value = 3200;
      ExTim_Freq_Current = EXACTO_TIM_3200;
      break;
    case 6400:
      value = 6400;
      ExTim_Freq_Current = EXACTO_TIM_6400;
      break;
    default:
      return;
      break;
  }
  uint32_t InitialAutoreload = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM3), value);
  LL_TIM_SetAutoReload(TIM3, InitialAutoreload);
  LL_TIM_GenerateEvent_UPDATE(TIM3);
}
void ex_frcTimReload()
{
  LL_TIM_GenerateEvent_UPDATE(TIM3);

}
exacto_tim_states_t  ex_getFreqHz_TIM()
{
  return ExTim_Freq_Current;
}
