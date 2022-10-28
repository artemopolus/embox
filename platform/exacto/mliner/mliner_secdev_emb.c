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
	if(mlimpl_runBoardIRQhandlerTX())
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
	if(mlimpl_runBoardIRQhandlerRX())
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
	mlimpl_enableBoard(MLINE_RXTX_BUFFER_SIZE, MLINE_RXTX_BUFFER_SIZE);
	SpiIsEnabled = 1;
}
void mldev_disable()
{
	irq_detach(15,NULL);
	irq_detach(14,NULL);
	mlimpl_disableBoard();
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
	mlimpl_initBoard();
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

	mlimpl_setBoardBuffer(&TransmitSpiDev, &ReceiveSpiDev);

	SpiIsEnabled = 0;
	return 0;
}
void mldev_setTxBuff(ExactoBufferUint8Type * buffer)
{
	TransmitSpiDev.storage = buffer;
}
void mldev_setRxBuff(ExactoBufferUint8Type * buffer)
{
	ReceiveSpiDev.storage = buffer;
}
void mldev_receive()
{
	if(SpiIsEnabled == 0)
	{
		initSpiDevSec();
		enableSpiDevSec();
	}
		if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	mlimpl_receiveBoard(&ReceiveSpiDev);
}
uint8_t mldev_repeatTransmit()
{
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		mlimpl_resetBoardRxTx(&ReceiveSpiDev, &TransmitSpiDev);
		return 1;
	}
	return 0;
}
uint8_t mldev_transmit()
{
	if(!ReceiveSpiDev.isready)
	{
		mlimpl_resetBoardRx(&ReceiveSpiDev);	
		return 0;
	}
	if(!ex_checkGpio(EX_GPIO_SPI_MLINE))
	{
		if(TransmitSpiDev.isready )
		{
			mlimpl_receiveTransmitBoard(&ReceiveSpiDev, &TransmitSpiDev);
			TransmitSpiDev.isready = 0;
			ReceiveSpiDev.isready = 0;	
			return 1;
		}
		else
		{
			mlimpl_receiveBoard(&ReceiveSpiDev);
		}
	}
	return 0;
}
uint16_t mldev_getReceivedData(uint8_t * trg, uint16_t trglen)
{
	if(downloadSpiDevSecData(ReceiveSpiDev.dmabufferlen))
		return grball_exbu8(ReceiveSpiDev.storage, trg);
	return 0;
}
uint16_t mldev_setTransmitData(uint8_t * src, uint16_t srclen)
{
	int i;
	for(i = 0; i < srclen; i++)
	{
		if(!pshsft_exbu8(TransmitSpiDev.storage, src[i]))
			break;
	}
	return i;
}
uint8_t mldev_getTransmitResult()
{
	return TransmitSpiDev.isready;
}
uint8_t mldev_getReceiveResult()
{
	return ReceiveSpiDev.isready;
}

