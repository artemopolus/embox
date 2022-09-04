#ifndef SPI_MLINE_BASE_H_
#define SPI_MLINE_BASE_H_


#include <stdint.h>
#include "commander/exacto_buffer.h"

typedef struct spi_mline_dev
{
	uint8_t isfull;
	uint8_t isready;
	uint32_t 	dmabufferlen;
	uint8_t * 	dmabufferdata;
	ExactoBufferUint8Type * storage;
	int ( *processData )(uint32_t len);
}spi_mline_dev_t;

#endif
