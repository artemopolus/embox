#include "main.h"
#include "usb_device.h"
#include "gpio.h"


#include <embox/unit.h>

/**
  * @brief  The application entry point.
  * @retval int
  */
EMBOX_UNIT_INIT(initUSBdeviceCDC);
int initUSBdeviceCDC(void)
{

  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  return 0;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

