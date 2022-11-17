#include "mliner/mliner_maindev_impl.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "stm32f7xx_hal.h"


#include <embox/unit.h>
#include <kernel/irq.h>

#ifndef MLINE_RXTX_BUFFER_SIZE
#define MLINE_RXTX_BUFFER_SIZE 512
#endif

static uint8_t RxBuffer[MLINE_RXTX_BUFFER_SIZE];
static uint8_t TxBuffer[MLINE_RXTX_BUFFER_SIZE];

static uint32_t TxLen;
static uint32_t RxLen;

static struct mliner_dev * MlinerDev;
struct mliner_dev_ops SpiMainDev;

static uint8_t receiverProcess(uint8_t* Buf, uint32_t Len)
{
    memcpy(RxBuffer, Buf, Len);
    MlinerDev->readyRx = 1;
	return 0 ;
}

static int updateRx(struct mliner_dev * dev)
{
    return 0;
}
static int updateTxRx(struct mliner_dev * dev)
{
    if(!dev->readyTx)
        return 0;
    dev->receiveDataProcess(RxBuffer, RxLen);
    dev->transmitDataProcess(TxBuffer, TxLen);
	CDC_Transmit_FS(TxBuffer, TxLen);

    dev->readyRx = 0;
    dev->readyTx = 1;
    return 1;
}
static int read(struct mliner_dev * dev, uint8_t * data, uint16_t len)
{
   return 0;
}
static int close (struct mliner_dev * dev)
{
    dev->is_opened = 0;
    return 0;
}
static int open(struct mliner_dev * dev, const struct mliner_dev_params * params)
{
    dev->is_opened = 1;
    TxLen = params->len;
    RxLen = params->len;
    return 0;
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* USER CODE END Error_Handler_Debug */
}

static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}
EMBOX_UNIT_INIT(initUsbMliner);
static int initUsbMliner(void)
{
    MX_GPIO_Init();
    MX_USB_DEVICE_Init();

	CDC_setReceiver_FS(receiverProcess);

    for(int i = 0; i < MLINER_BASEDEV_DEVSBLOCK_CNT; i++)
    {
        if(MlinerDevsBlock.devices[i].is_enabled == 0)
        {
            MlinerDev = &MlinerDevsBlock.devices[i];
            MlinerDev->is_enabled = 1;
            MlinerDev->driver = &SpiMainDev;
            MlinerDev->driver->open = open;
            MlinerDev->driver->close = close;
            MlinerDev->driver->read = read;
            MlinerDev->driver->updateRx = updateRx;
            MlinerDev->driver->updateTxRx = updateTxRx;
            struct mliner_dev_params params;
            params.len = MLINE_RXTX_BUFFER_SIZE;
            open(MlinerDev, &params);

            break;
        }
    }

    return 0;
}
