#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_tim.h"
#include "system_stm32f1xx.h"
#include "stm32f1xx_ll_crc.h"
#include "stm32f1xx.h"

#include <embox/unit.h>

#include "hardtools/basic.h"

EMBOX_UNIT_INIT(apollon_hardtools_init);
static int apollon_hardtools_init(void)
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
  return 0;
}
uint8_t ex_feedCRC(uint8_t * buffer, uint16_t buffer_length)
{
  register uint32_t data = 0;
  register uint32_t index = 0;
  for ( index = 0; index < buffer_length; index += 4)
  {
    data =  (uint32_t)(buffer[4*index + 3] << 24);
    data |= (uint32_t)(buffer[4*index + 2] << 16);
    data |= (uint32_t)(buffer[4*index + 1] << 8);
    data |= (uint32_t)(buffer[4*index] );
    LL_CRC_FeedData32(CRC, data);
  }
  
  return 0;
}
uint32_t ex_getResultCRC()
{
  return(LL_CRC_ReadData32(CRC));
}
uint8_t  ex_getCRC(uint8_t * buffer, uint16_t buffer_length, uint32_t * result)
{
  return 0;
}

