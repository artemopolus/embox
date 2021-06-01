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

static uint32_t HTBS1_CRC_buf;
static uint8_t  HTBS1_CRC_cnt = 0;
static uint8_t  HTBS1_CRC_max = 4;

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
  for ( index = 0; (index + 3) < buffer_length; index += 4)
  {
    data =  (uint32_t)(buffer[index + 3] << 24);
    data |= (uint32_t)(buffer[index + 2] << 16);
    data |= (uint32_t)(buffer[index + 1] << 8);
    data |= (uint32_t)(buffer[index] );
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
  ex_feedCRC(buffer, buffer_length);
  *result = ex_getResultCRC();
  return 0;
}
uint8_t ex_updateCRC(uint8_t value)
{
  if (HTBS1_CRC_cnt == 0)
  {
    HTBS1_CRC_buf = (uint32_t)(value);
  }
  else if (HTBS1_CRC_cnt == 1)
  {
    HTBS1_CRC_buf |= (uint32_t)(value << 8);
  }
  else if (HTBS1_CRC_cnt == 2)
  {
    HTBS1_CRC_buf |= (uint32_t)(value << 16);
  }
  else if (HTBS1_CRC_cnt == 3)
  {
    HTBS1_CRC_buf |= (uint32_t)(value << 24);
    LL_CRC_FeedData32(CRC, HTBS1_CRC_buf);
  }
  if(HTBS1_CRC_cnt < HTBS1_CRC_max)
  {
    HTBS1_CRC_cnt++;
  }
  else
  {
    HTBS1_CRC_cnt = 0;
  }
  return 0;
}

