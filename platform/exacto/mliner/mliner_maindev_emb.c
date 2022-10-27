#include "mliner/mliner_maindev.h"
#include "mliner/mliner_maindev_impl.h"

#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/lthread/lthread.h>


#include "gpio_config.h"
#include "gpio/gpio_spi.h"

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
        ex_disableGpio(EX_GPIO_SPI_MLINE); //теперь можно обновлять tx на аполлоне
    }
    return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(SPI_MLINE_DMA_IRQ_TX, TransmitIRQhandler, NULL);
static irq_return_t ReceiveIRQhandler(unsigned int irq_nr, void *data)
{
	if(runBoardMlDevIRQhandlerRX())
	{
		ReceiveSpiDev.isready = 1;
	} 
   return IRQ_HANDLED;
}
STATIC_IRQ_ATTACH(SPI_MLINE_DMA_IRQ_RX, ReceiveIRQhandler, NULL);

void enableSpiDevSec()
{
    irq_attach(SPI_MLINE_DMA_IRQ_TX, TransmitIRQhandler, 0, NULL, "TransmitIRQhandler");
    irq_attach(SPI_MLINE_DMA_IRQ_RX, ReceiveIRQhandler, 0, NULL, "ReceiveIRQhandler");
    enableBoardMlDev(MLINE_RXTX_BUFFER_SIZE, MLINE_RXTX_BUFFER_SIZE);
	SpiIsEnabled = 1;
}
void disableSpiDevSec()
{
	irq_detach(SPI_MLINE_DMA_IRQ_TX,NULL);
	irq_detach(SPI_MLINE_DMA_IRQ_RX,NULL);
	disableBoardMlDev();
	SpiIsEnabled = 0;
}
int downloadSpiDevSecData(uint32_t len)
{
    if(!ReceiveSpiDev.isready)
        return 0;
   //  pshsftPack_exbu8(ReceiveSpiDev.storage, ReceiveSpiDev.dmabufferdata, ReceiveSpiDev.dmabufferlen);

	if(ReceiveSpiDev.collect_on)
		ReceiveSpiDev.collect(ReceiveSpiDev.dmabufferdata, len);

	for (int i = 0; i < ReceiveSpiDev.dmabufferlen; i++)
	{
		ReceiveSpiDev.dmabufferdata[i] = 0;
	}
	
	return 1;
}
int uploadSpiDevSecData(uint32_t len)
{
	// for(uint32_t i = 0; i < TransmitSpiDev.dmabufferlen; i++)
	// {
	// 	if(!grbfst_exbu8(TransmitSpiDev.storage, &TransmitSpiDev.dmabufferdata[i]))
	// 		break;
	// }

	if(TransmitSpiDev.collect_on)
	{
		TransmitSpiDev.collect(TransmitSpiDev.dmabufferdata, len);
	}

   ex_enableGpio(EX_GPIO_SPI_MLINE); //block update apollon tx

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
void setTxBuffMlinerDev(ExactoBufferUint8Type * buffer)
{
	TransmitSpiDev.storage = buffer;
}
void setRxBuffMlinerDev(ExactoBufferUint8Type * buffer)
{
	ReceiveSpiDev.storage = buffer;
}
void receiveSpiDevSec()
{
	if(SpiIsEnabled == 0)
	{
		initSpiDevSec();
		enableSpiDevSec();
	}
	receiveBoardMlDev(&ReceiveSpiDev);
}
uint8_t repeatTransmitSpiDevSec()
{
	resetBoardMlDevRxTx(&ReceiveSpiDev, &TransmitSpiDev);
    TransmitSpiDev.isready = 0;
    ReceiveSpiDev.isready = 0;	
	return 1;
}
uint8_t transmitMlinerDev()
{
	if(SpiIsEnabled == 0)
	{
		initSpiDevSec();
		enableSpiDevSec();
	}

    if(TransmitSpiDev.isready )
    {
        receiveTransmitBoardMlDev(&ReceiveSpiDev, &TransmitSpiDev);
        TransmitSpiDev.isready = 0;
        ReceiveSpiDev.isready = 0;	
        return 1;
    }
	return 0;
}
uint16_t getReceivedDataSpiDevSec(uint8_t * trg, uint16_t trglen)
{
	if(downloadSpiDevSecData(ReceiveSpiDev.dmabufferlen))
		return grball_exbu8(ReceiveSpiDev.storage, trg);
	return 0;
}
uint16_t setTransmitDataSpiDevSec(uint8_t * src, uint16_t srclen)
{
	int i;
	for(i = 0; i < srclen; i++)
	{
		if(!pshsft_exbu8(TransmitSpiDev.storage, src[i]))
			break;
	}
	return i;
}
void setReceiverMlinerDev(int(*receiveProcess)(uint8_t * data, uint16_t datalen))
{
	ReceiveSpiDev.collect = receiveProcess;
	ReceiveSpiDev.collect_on = 1;
}
void setTransmitMlinerDev(int(*transmitProcess)(uint8_t * data, uint16_t datalen))
{
	TransmitSpiDev.collect = transmitProcess;
	TransmitSpiDev.collect_on = 1;
}
uint8_t getTransmitResultMlinerDev()
{
	return TransmitSpiDev.isready;
}
uint8_t getReceiveResultMlinerDev()
{
	return ReceiveSpiDev.isready;
}

