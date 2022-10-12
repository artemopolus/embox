/**
  ******************************************************************************
  * @file    USB_Device/MSC_Standalone/Src/usbd_storage.c
  * @author  MCD Application Team
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage.h"
#include "stm32746g_discovery_sd.h"
#include <kernel/irq.h>
#include <util/log.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  0x10000
#define STORAGE_BLK_SIZ                  0x200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t writestatus, readstatus = 0;

/* USB Mass storage Standard Inquiry Data */
int8_t STORAGE_Inquirydata[] = { /* 36 */
  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer: 8 bytes  */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product     : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0','1',                      /* Version     : 4 Bytes  */
};

/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  STORAGE_Inquirydata,
};
/* Private functions ---------------------------------------------------------*/

extern SD_HandleTypeDef uSdHandle;
	// option number dma_sdmmc_irq = 49
	// option number dma_rx_irq    = 59
	// option number dma_tx_irq    = 69

#define STM32_DMA_RX_IRQ 59
#define STM32_DMA_TX_IRQ 69
#define STM32_SDMMC_IRQ 49

static irq_return_t stm32_dma_rx_irq(unsigned int irq_num, void *dev) {
	HAL_DMA_IRQHandler(uSdHandle.hdmarx);
	return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(STM32_DMA_RX_IRQ, stm32_dma_rx_irq, NULL);

static irq_return_t stm32_dma_tx_irq(unsigned int irq_num, void *dev) {
	HAL_DMA_IRQHandler(uSdHandle.hdmatx);
	return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(STM32_DMA_TX_IRQ, stm32_dma_tx_irq, NULL);

static irq_return_t stm32_sdmmc_irq(unsigned int irq_num, void *dev) {
	HAL_SD_IRQHandler(&uSdHandle);
	return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(STM32_SDMMC_IRQ, stm32_sdmmc_irq, NULL);




/**
  * @brief  Initializes the storage unit (medium)
  * @param  lun: Logical unit number
  * @retval Status (0 : OK / -1 : Error)
  */
int8_t STORAGE_Init(uint8_t lun)
{
	// if (!block_dev_lookup(STM32F7_SD_DEVNAME)) {
	// 	log_error("Block device not found");
	// 	return -1;
	// }
  if (BSP_SD_Init() == MSD_OK) {
    if (0 != irq_attach(STM32_DMA_RX_IRQ,
          stm32_dma_rx_irq,
          0, NULL, "stm32_dma_rx_irq")) {
      log_error("irq_attach error");
      return -1;
    }
    if (0 != irq_attach(STM32_DMA_TX_IRQ,
          stm32_dma_tx_irq,
          0, NULL, "stm32_dma_tx_irq")) {
      log_error("irq_attach error");
      return -1;
    }
    if (0 != irq_attach(STM32_SDMMC_IRQ,
          stm32_sdmmc_irq,
          0, NULL, "stm32_sdmmc_irq")) {
      log_error("irq_attach error");
      return -1;
    }
  	/* SDMMC2 irq priority should be higher that DMA due to
	 * STM32Cube implementation. */
	  irqctrl_set_prio(STM32_DMA_RX_IRQ, 10);
	  irqctrl_set_prio(STM32_DMA_TX_IRQ, 10);
	  irqctrl_set_prio(STM32_SDMMC_IRQ, 11);
 		return 0;
	} else if (BSP_SD_IsDetected() != SD_PRESENT) {
		/* microSD Card is not inserted, do nothing. */
		return 0;
	} else {
		log_error("BSP_SD_Init error");
		return -1;
	}
 return 0;
}

/**
  * @brief  Returns the medium capacity.
  * @param  lun: Logical unit number
  * @param  block_num: Number of total block number
  * @param  block_size: Block size
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  HAL_SD_CardInfoTypeDef info;
  int8_t ret = -1;

  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    BSP_SD_GetCardInfo(&info);

    *block_num = info.LogBlockNbr;
    *block_size = info.LogBlockSize;
    ret = 0;
  }
  return ret;
}

/**
  * @brief  Checks whether the medium is ready.
  * @param  lun: Logical unit number
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
  static int8_t prev_status = 0;
  int8_t ret = -1;

  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    if(prev_status < 0)
    {
      BSP_SD_Init();
      prev_status = 0;

    }
    if(BSP_SD_GetCardState() == SD_TRANSFER_OK)
    {
      ret = 0;
    }
  }
  else if(prev_status == 0)
  {
    prev_status = -1;
  }

  return ret;
}

/**
  * @brief  Checks whether the medium is write protected.
  * @param  lun: Logical unit number
  * @retval Status (0: write enabled / -1: otherwise)
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  return 0;
}

/**
  * @brief  Reads data from the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0: OK / -1: Error)
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = -1;

  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    ipl_t ipl;
    ipl = ipl_save();
    BSP_SD_ReadBlocks_DMA((uint32_t *)buf, blk_addr, blk_len);
    ipl_restore(ipl);

    /* Wait for Rx Transfer completion */
    // while (readstatus == 0)
    // {
    // }
    // readstatus = 0;

    /* Wait until SD card is ready to use for new operation */
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
    {
    }

    ret = 0;
  }
  return ret;
}

/**
  * @brief  Writes data into the medium.
  * @param  lun: Logical unit number
  * @param  blk_addr: Logical block address
  * @param  blk_len: Blocks number
  * @retval Status (0 : OK / -1 : Error)
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  int8_t ret = -1;

  if(BSP_SD_IsDetected() != SD_NOT_PRESENT)
  {
    BSP_SD_WriteBlocks_DMA((uint32_t *)buf, blk_addr, blk_len);

    /* Wait for Tx Transfer completion */
    while (writestatus == 0)
    {
    }
    writestatus = 0;

    /* Wait until SD card is ready to use for new operation */
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
    {
    }

    ret = 0;
  }
  return ret;
}

/**
  * @brief  Returns the Max Supported LUNs.
  * @param  None
  * @retval Lun(s) number
  */
int8_t STORAGE_GetMaxLun(void)
{
  return(STORAGE_LUN_NBR - 1);
}

/**
  * @brief BSP Tx Transfer completed callbacks
  * @param None
  * @retval None
  */
void BSP_SD_WriteCpltCallback(void)
{
  writestatus = 1;
}

/**
  * @brief BSP Rx Transfer completed callbacks
  * @param None
  * @retval None
  */
void BSP_SD_ReadCpltCallback(void)
{
  readstatus = 1;
}



