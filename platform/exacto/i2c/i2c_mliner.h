#ifndef I2C_MLINER_H
#define I2C_MLINER_H
#include <stdint.h>
#include <errno.h>
#include <embox/unit.h>
#include <kernel/irq.h>
#include <kernel/printk.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>

#define BUFFER_SIZE_I2C_MLINER EXACTOLINK_MESSAGE_SIZE

extern void enable_I2C_MLINER(void);
extern void bind_TX_thread_I2C_MLINER(int (*run)(struct lthread *));
extern void bind_RX_thread_I2C_MLINER(int (*run)(struct lthread *));
extern void transmit_I2C_MLINER(uint8_t * data, const uint16_t datalen, uint32_t address);
extern void receive_I2C_MLINER(uint8_t * data, const uint16_t datalen);
#endif //
