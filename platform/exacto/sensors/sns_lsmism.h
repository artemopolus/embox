#ifndef EXACTO_SENSORS_SNS_LSMISM_H
#define EXACTO_SENSORS_SNS_LSMISM_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "commander/exacto_data_storage.h"

extern uint32_t Apollon_lsmism_MlineReceive;
//extern uint32_t Apollon_lsmism_MlineTransmit;
extern uint32_t Apollon_lsmism_MlineOverFlow;
extern volatile uint8_t Apollon_lsmism_MlineRXTx_Readable;
extern uint32_t Apollon_lsmism_Ticker_Buf;
extern volatile uint8_t Apollon_lsmism_Ticker_Readable;
extern volatile uint8_t Apollon_lsmism_Buffer_Readable;

extern volatile int16_t Apollon_lsmism_Buffer_Data0[3];
extern volatile int16_t Apollon_lsmism_Buffer_Data1[3];
extern volatile int16_t Apollon_lsmism_Buffer_Data2[3];

extern uint8_t exSnsStart(const uint8_t type);
extern uint8_t exSnsStop(void);

#endif