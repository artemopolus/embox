

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include <embox/unit.h>
/* Private typedef -----------------------------------------------------------*/
typedef enum 
{
  SHIELD_NOT_DETECTED = 0, 
  SHIELD_DETECTED
}ShieldStatus;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USBD_HandleTypeDef USBD_Device;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
EMBOX_UNIT_INIT(initUSBDeviceMSC);
int initUSBDeviceMSC(void)
{
  HAL_Init();
  /* Init Device Library */
  USBD_Init(&USBD_Device, &MSC_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);
  
  /* Add Storage callbacks for MSC Class */
  USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);
  return 0; 
}
void HAL_Delay(__IO uint32_t Delay)
{
  while(Delay)
  {
    if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    {
      Delay--;
    }
  }
}


