#include "mliner/mliner_secdev.h"
#include "mliner/mliner_secdev_impl.h"

#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>


#include "gpio/gpio.h"

uint8_t ReceiveBuffer[MLINE_RXTX_BUFFER_SIZE];
uint8_t TransmitBuffer[MLINE_RXTX_BUFFER_SIZE];


spi_mline_dev_t TransmitSpiDev;
spi_mline_dev_t ReceiveSpiDev;

static uint8_t SpiIsEnabled = 0;

static irq_return_t TransmitIRQhandler(unsigned int irq_nr, void *data)
{
	if(runBoardMlDevIRQhandlerTX())
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
	if(runBoardMlDevIRQhandlerRX())
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
	enableBoardMlDev(MLINE_RXTX_BUFFER_SIZE, MLINE_RXTX_BUFFER_SIZE);
	SpiIsEnabled = 1;
}
void disableDevSec()
{
	irq_detach(15,NULL);
	irq_detach(14,NULL);
	disableBoardMlDev();
	SpiIsEnabled = 0;
}
int downloadSpiDevSecData(uint32_t len)
{
	if(!ReceiveSpiDev.isready)
		return 0;
   pshsftPack_exbu8(ReceiveSpiDev.storage, ReceiveSpiDev.dmabufferdata, ReceiveSpiDev.dmabufferlen);
	
   for (int i = 0; i < ReceiveSpiDev.dmabufferlen; i++)
   {
      ReceiveSpiDev.dmabufferdata[i] = 0;
   }
   
   
   // for(uint32_t i = 0; i < ReceiveSpiDev.dmabufferlen; i++)
	// {
	// 	if(!pshsft_exbu8(ReceiveSpiDev.storage, ReceiveSpiDev.dmabufferdata[i]))
	// 	{
	// 		break;
	// 	}
	// }
	return 1;
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
	initBoardMlDev();
	TransmitSpiDev.dmabufferdata = TransmitBuffer;
	TransmitSpiDev.dmabufferlen = MLINE_RXTX_BUFFER_SIZE;
	TransmitSpiDev.processData = uploadSpiDevSecData;
	// TransmitSpiDev.storage = &TransmitStore;
	TransmitSpiDev.isfull = 0;
	TransmitSpiDev.isready = 1;

	ReceiveSpiDev.dmabufferdata = ReceiveBuffer;
	ReceiveSpiDev.dmabufferlen = MLINE_RXTX_BUFFER_SIZE;
	ReceiveSpiDev.processData = downloadSpiDevSecData;
	// ReceiveSpiDev.storage = &ReceiveStore;
	ReceiveSpiDev.isfull = 0;
	ReceiveSpiDev.isready = 1;

	setBoardMlDevBuffer(&TransmitSpiDev, &ReceiveSpiDev);

	SpiIsEnabled = 0;
	return 0;
}
void setTxBuffDevSec(ExactoBufferUint8Type * buffer)
{
	TransmitSpiDev.storage = buffer;
}
void setRxBuffDevSec(ExactoBufferUint8Type * buffer)
{
	ReceiveSpiDev.storage = buffer;
}
void receiveDevSec()
{
	if(SpiIsEnabled == 0)
	{
		initSpiDevSec();
		enableSpiDevSec();
	}
		if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	receiveBoardMlDev(&ReceiveSpiDev);
}
uint8_t repeatTransmitDevSec()
{
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		resetBoardMlDevRxTx(&ReceiveSpiDev, &TransmitSpiDev);
		return 1;
	}
	return 0;
}
uint8_t transmitDevSec()
{
	if(!ReceiveSpiDev.isready)
	{
		resetBoardMlDevRx(&ReceiveSpiDev);	
		return 0;
	}
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		if(TransmitSpiDev.isready )
		{
			receiveTransmitBoardMlDev(&ReceiveSpiDev, &TransmitSpiDev);
			TransmitSpiDev.isready = 0;
			ReceiveSpiDev.isready = 0;	
			return 1;
		}
		else
		{
			receiveBoardMlDev(&ReceiveSpiDev);
		}
	}
	return 0;
}
uint16_t getReceivedDataDevSec(uint8_t * trg, uint16_t trglen)
{
	if(downloadSpiDevSecData(ReceiveSpiDev.dmabufferlen))
		return grball_exbu8(ReceiveSpiDev.storage, trg);
	return 0;
}
uint16_t setTransmitDataDevSec(uint8_t * src, uint16_t srclen)
{
	int i;
	for(i = 0; i < srclen; i++)
	{
		if(!pshsft_exbu8(TransmitSpiDev.storage, src[i]))
			break;
	}
	return i;
}
uint8_t getTransmitResultDevSec()
{
	return TransmitSpiDev.isready;
}
uint8_t getReceiveResultDevSec()
{
	return ReceiveSpiDev.isready;
}

