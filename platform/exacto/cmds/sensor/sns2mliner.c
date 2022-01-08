#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "commander/exacto_data_storage.h"
#include "commander/exacto_sns_ctrl.h"
#include "spi/spi_sns.h"
#include "tim/tim.h"
#include <embox/unit.h>
#include <hal/reg.h>

#include "kernel/printk.h"
#include "sensors/ism330dlc_reg.h"
#include "sensors/lsm303ah_reg.h"
#include "sensor/sns_service.h"
#define PRINT_TICKER_MAX 9 
#define TRANSMIT_MESSAGE_SIZE EXACTOLINK_MESSAGE_SIZE

// #define SNS_SERVICE_TESTING

ex_sns_lth_container_t SnsContainer;

#ifdef SNS_SERVICE_TESTING

#define DEMCR        0xE000EDFC
#define DEMCR_TRCENA    0x01000000
#define DWT_LAR      0xE0001FB0
#define DWT_LAR_KEY  0xC5ACCE55
#define DWT_CYCCNT   0xE0001004
#define DWT_CTRL     0xE0001000
# define CYCCNTENA   (1 << 0)

static void dwt_cyccnt_reset(void) {
	REG32_ORIN(DEMCR, DEMCR_TRCENA);

	REG32_STORE(DWT_LAR, DWT_LAR_KEY);

	REG32_STORE(DWT_CYCCNT, 0);
}
static inline uint32_t dwt_cyccnt_start(void) {
	REG32_ORIN(DWT_CTRL, CYCCNTENA);

	return REG32_LOAD(DWT_CYCCNT);
}

static inline uint32_t dwt_cyccnt_stop(void) {
	REG32_CLEAR(DWT_CTRL, CYCCNTENA);

	return REG32_LOAD(DWT_CYCCNT);
}
#endif

int main(int argc, char *argv[]) {
	return 0;
}

