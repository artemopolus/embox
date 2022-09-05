#include "spi_mline_sec_impl.h"
#include "spi_mline_sec.h"

#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>


#include "gpio/gpio.h"

uint8_t ReceiveBuffer[SPI_MLINE_RXTX_BUFFER_SIZE];
uint8_t TransmitBuffer[SPI_MLINE_RXTX_BUFFER_SIZE];


spi_mline_dev_t TransmitSpiDev;
spi_mline_dev_t ReceiveSpiDev;

static uint8_t SpiIsEnabled = 0;

static irq_return_t TransmitIRQhandler(unsigned int irq_nr, void *data)
{
	if(runBoardSpiIRQhandlerTX())
	{
		TransmitSpiDev.isready = 1;
	// 	    SPI2_FULL_DMA_tx_buffer.is_full = 0;
   //  ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 1;
	}
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(15, TransmitIRQhandler, NULL);
static irq_return_t ReceiveIRQhandler(unsigned int irq_nr, void *data)
{
	if(runBoardSpiIRQhandlerRX())
	{
		ReceiveSpiDev.isready = 1;
	// 	    SPI2_FULL_DMA_rx_buffer.is_full = 1;
   //  ExDtStr_Output_Storage[EX_THR_SPi_RX].isready = 1;
   //  ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_WAIT;
   //  ex_updateCounter_ExDtStr(EX_THR_SPi_RX);
	} 
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(14, ReceiveIRQhandler, NULL);

void enableSpiDevSec()
{
   irq_attach(15, TransmitIRQhandler, 0, NULL, "TransmitIRQhandler");
   irq_attach(14, ReceiveIRQhandler, 0, NULL, "ReceiveIRQhandler");
	enableBoardSpi(SPI_MLINE_RXTX_BUFFER_SIZE, SPI_MLINE_RXTX_BUFFER_SIZE);
	SpiIsEnabled = 1;
}
void disableSpiDevSec()
{
	irq_detach(15,NULL);
	irq_detach(14,NULL);
	disableBoardSpi();
	SpiIsEnabled = 0;
}
int downloadSpiDevSecData(uint32_t len)
{
   pshsftPack_exbu8(ReceiveSpiDev.storage, ReceiveSpiDev.dmabufferdata, ReceiveSpiDev.dmabufferlen);
	// for(uint32_t i = 0; i < ReceiveSpiDev.dmabufferlen; i++)
	// {
	// 	if(!pshsft_exbu8(ReceiveSpiDev.storage, ReceiveSpiDev.dmabufferdata[i]))
	// 	{
	// 		break;
	// 	}
	// }
	return 0;
}
int uploadSpiDevSecData(uint32_t len)
{
	for(uint32_t i = 0; i < TransmitSpiDev.dmabufferlen; i++)
	{
		if(!grbfst_exbu8(TransmitSpiDev.storage, &TransmitSpiDev.dmabufferdata[i]))
			break;
	}
	return 0;
}
EMBOX_UNIT_INIT(initSpiDevSec);
static int initSpiDevSec(void)
{
	initBoardSpi();
	TransmitSpiDev.dmabufferdata = TransmitBuffer;
	TransmitSpiDev.dmabufferlen = SPI_MLINE_RXTX_BUFFER_SIZE;
	TransmitSpiDev.processData = uploadSpiDevSecData;
	// TransmitSpiDev.storage = &TransmitStore;
	TransmitSpiDev.isfull = 0;
	TransmitSpiDev.isready = 1;

	ReceiveSpiDev.dmabufferdata = ReceiveBuffer;
	ReceiveSpiDev.dmabufferlen = SPI_MLINE_RXTX_BUFFER_SIZE;
	ReceiveSpiDev.processData = downloadSpiDevSecData;
	// ReceiveSpiDev.storage = &ReceiveStore;
	ReceiveSpiDev.isfull = 0;
	ReceiveSpiDev.isready = 1;

	setBoardSpiBuffer(&TransmitSpiDev, &ReceiveSpiDev);

	SpiIsEnabled = 0;
	return 0;
}
void setTxBuffSpiDevSec(ExactoBufferUint8Type * buffer)
{
	TransmitSpiDev.storage = buffer;
}
void setRxBuffSpiDevSec(ExactoBufferUint8Type * buffer)
{
	ReceiveSpiDev.storage = buffer;
}
void receiveSpiDevSec()
{
	//     if(exds_isNeedToReset())
   //  {
   //      turnOffSPI2_FULL_DMA();
   //      exds_resetInterface(0);
   //      // HAL_NVIC_SystemReset(); <= hard reset to MCU
   //      return 0;
   //  }


   //  ex_thread_control_t * _trg_thread;
   //  _trg_thread = (ex_thread_control_t *)self;
   //  const uint32_t _datacount = SPI2_FULL_DMA_rx_buffer.dt_count ;

   //  if(SpiIsEnabled == 0)
   //  {
   //      // setupSpiReceiveSlave();
   //      SPI2_FULL_DMA_init();
   //      setupSPI2_FULL_DMA();
   //  }
	if(SpiIsEnabled == 0)
	{
		initSpiDevSec();
		enableSpiDevSec();
	}
   //  // if (SPI2_FULL_DMA_rx_buffer.is_full == 0)
   //  //     return 1;
	receiveBoardSpi(&ReceiveSpiDev);
   //  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
   //  for (uint16_t i = 0; i < _datacount; i++)
   //  {
   //      pshfrc_exbu8(&_trg_thread->datastorage, SPI2_FULL_DMA_rx_buffer.dt_buffer[i]);
   //      SPI2_FULL_DMA_rx_buffer.dt_buffer[i] = 0;
   //  }
   //  SPI2_FULL_DMA_rx_buffer.is_full = 0;
   //  _trg_thread->isready = 0;
   //  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, _datacount);
   //  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
   //  return 0;
}
void transmitSpiDevSec()
{
	// exactolink_package_result_t res = (exactolink_package_result_t) exds_getStatus(EX_THR_SPi_TX);
   //  //если требований от мастера не поступало, продолжаем прослушивание
   //  if (res == EXACTOLINK_NO_DATA)
   //  {
   //      if (!ExDtStr_Output_Storage[EX_THR_SPi_RX].isready&&!ex_checkGpio(EX_GPIO_SPI_MLINE))  //можно ли обновлять
   //      {
   //          LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
   //          LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
   //          LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
   //      }
   //      return 0;
   //  }
	if(!ReceiveSpiDev.isready)
	{
		resetBoardSpiRx(&ReceiveSpiDev);	
		return;
	}
   //  //Требуется повтор передачи данных
   //  if(res == EXACTOLINK_REPEAT)
   //  {
   //      repeatSend();
   //      return 0;
   //  }
   //  //Пришел запрос
   //  if  (
   //          (ExDtStr_Output_Storage[EX_THR_SPi_RX].result == EX_THR_CTRL_OK)
   //      )
   //  {
   //      ExDtStr_Output_Storage[EX_THR_SPi_RX].result = EX_THR_CTRL_NO_RESULT;
   //  }
   //  if (
   //      (ExDtStr_Output_Storage[EX_THR_SPi_TX].result == EX_THR_CTRL_OK)
   //      &&(ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
   //      )
   //  {
   //      //данные готовы к отправке и шлюз свободен
   //      if (!ex_checkGpio(EX_GPIO_SPI_MLINE))  //можно ли обновлять
   //      {
   //          SPI2_disableChannels();
   //          getMailFromExactoDataStorage(SPI2_FULL_DMA_tx_buffer.dt_buffer, SPI2_FULL_DMA_tx_buffer.dt_count);
   //          ExDtStr_Output_Storage[EX_THR_SPi_TX].isready = 0;
   //          ExDtStr_Output_Storage[EX_THR_SPi_TX].result = EX_THR_CTRL_INIT;
   //          ex_updateCounter_ExDtStr(EX_THR_SPi_TX);
   //          SPI2_updateRx();
   //          SPI2_enableChannels();
   //      }
   //  }
	if(TransmitSpiDev.isready && (!ex_checkGpio(EX_GPIO_SPI_MLINE)))
	{
		receiveTransmitBoardSpi(&ReceiveSpiDev, &TransmitSpiDev);
		TransmitSpiDev.isready = 0;
		ReceiveSpiDev.isready = 0;	
	}
   //  else if (
   //      (ExDtStr_Output_Storage[EX_THR_SPi_TX].result != EX_THR_CTRL_OK)
   //      &&(ExDtStr_Output_Storage[EX_THR_SPi_TX].isready)
   //      )
   //  {
   //      // данные не готовы, но шлюз свободен : ничего не делать, только принимать что-либо
   //      LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4); //receive
   //      SPI2_updateRx();
   //      LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, SPI2_FULL_DMA_RXTX_BUFFER_SIZE);
   //      LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);   //receive
   //  }
   //  else // ! ExDtStr_Output_Storage[EX_THR_SPi_TX].isready
   //  {
   //      //шлюз занят, готовность данных не имеет значения : повторить отправку
   //      repeatSend();
   //  }
	else
	{
		if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
		{
			receiveBoardSpi(&ReceiveSpiDev);
		}
	}
   //  return 0;
}