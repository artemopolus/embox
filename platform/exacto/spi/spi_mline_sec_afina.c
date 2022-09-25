#include "spi_mline_sec_impl.h"

#include "gpio_config.h"

int initBoardSpi(void)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    //LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    SPI_MLINE_ENABLE_CLOCK_SPI

    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    //LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    SPI_MLINE_ENABLE_GPIO
    /**SPI1 GPIO Configuration
     PA5   ------> SPI1_SCK
    PA7   ------> SPI1_MOSI
    PB4   ------> SPI1_MISO
    */
    GPIO_InitStruct.Pin = SPI_MLINE_SCK_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_SCK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_MLINE_MOSI_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_MOSI_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_MLINE_MISO_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType =    LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(SPI_MLINE_MISO_PORT, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_RX Init */
    LL_DMA_SetChannelSelection      (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, SPI_MLINE_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel   (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode          (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode  (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);

      /* SPI1_TX Init */
    LL_DMA_SetChannelSelection      (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, SPI_MLINE_DMA_CHANNEL);
    LL_DMA_SetDataTransferDirection (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel   (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode          (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode  (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);

    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 10;
    LL_SPI_Init(SPI_MLINE_SPI, &SPI_InitStruct);

    return 0;
}

void setBoardSpiBuffer(
	spi_mline_dev_t * transmit, spi_mline_dev_t * receive
	// uint8_t * rxdata, uint32_t rxlen, uint8_t * txdata, uint32_t txlen, int (*download)(uint32_t len), int(*upload)(uint32_t len)
	)
{
    LL_DMA_ConfigAddresses(SPI_MLINE_DMA, 
                            SPI_MLINE_DMA_STREAM_RX,
                           LL_SPI_DMA_GetRegAddr(SPI_MLINE_SPI),
							(uint32_t)receive->dmabufferdata,
                           LL_DMA_GetDataTransferDirection(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX));
    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, receive->dmabufferlen );

    LL_DMA_ConfigAddresses(SPI_MLINE_DMA,
                           SPI_MLINE_DMA_STREAM_TX, 
							(uint32_t)transmit->dmabufferdata,
                           LL_SPI_DMA_GetRegAddr(SPI_MLINE_SPI),
                           LL_DMA_GetDataTransferDirection(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX));
    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, transmit->dmabufferlen);

}

uint8_t runBoardSpiIRQhandlerRX(void)
{
    if (LL_DMA_IsActiveFlag_TC0(SPI_MLINE_DMA) != RESET)
    {
        LL_DMA_ClearFlag_TC0(SPI_MLINE_DMA);
        return 1;
    }
    return 0;
}
uint8_t runBoardSpiIRQhandlerTX(void)
{
    if (LL_DMA_IsActiveFlag_TC5(SPI_MLINE_DMA) != RESET)
    {
        LL_DMA_ClearFlag_TC5(SPI_MLINE_DMA);
        return 1;
    }
    return 0;
}
void enableBoardSpi(uint32_t rxlen, uint32_t txlen)
{
    LL_DMA_EnableIT_TC(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);
    LL_DMA_EnableIT_TE(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX);
    LL_DMA_EnableIT_TC(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);
    LL_DMA_EnableIT_TE(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX);

    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, txlen); //устанавливаем сколько символов передачть
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, rxlen); //устанавливаем сколько символов передачть
    LL_SPI_EnableDMAReq_RX(SPI_MLINE_SPI);
    LL_SPI_EnableDMAReq_TX(SPI_MLINE_SPI);
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    LL_SPI_Enable(SPI_MLINE_SPI);
}
void disableBoardSpi(void)
{
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 
    LL_SPI_Disable(SPI_MLINE_SPI);

}
void receiveBoardSpi(spi_mline_dev_t * receiver)
{
    resetBoardSpiRx(receiver);
}
void resetBoardSpiRx(spi_mline_dev_t * receiver)
{
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, receiver->dmabufferlen); //устанавливаем сколько символов передачть
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
}
void resetBoardSpiRxTx(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit)
{
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 


    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, transmit->dmabufferlen);
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, receiver->dmabufferlen); //устанавливаем сколько символов передачть

    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 

}

void receiveTransmitBoardSpi(spi_mline_dev_t * receiver, spi_mline_dev_t * transmit)
{
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_DisableStream(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 

    if(receiver->collect_on)
        receiver->collect(receiver->dmabufferdata, receiver->dmabufferlen);
	transmit->processData(transmit->dmabufferlen);

    LL_DMA_SetDataLength(SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX, transmit->dmabufferlen);
    LL_DMA_SetDataLength    (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX, receiver->dmabufferlen); //устанавливаем сколько символов передачть

    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_RX); //enable receive
    LL_DMA_EnableStream     (SPI_MLINE_DMA, SPI_MLINE_DMA_STREAM_TX); //enable transmit 

}