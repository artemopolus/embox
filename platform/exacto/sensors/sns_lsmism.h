#ifndef EXACTO_SENSORS_SNS_LSMISM_H
#define EXACTO_SENSORS_SNS_LSMISM_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern uint32_t Apollon_lsmism_MlineReceive;
extern uint32_t Apollon_lsmism_MlineTransmit;
extern uint8_t Apollon_lsmism_MlineRXTx_Readable;
extern uint32_t Apollon_lsmism_Ticker_Buf;
extern uint8_t Apollon_lsmism_Ticker_Readable;

extern uint8_t exSnsStart(void);
extern uint8_t exSnsStop(void);

#endif