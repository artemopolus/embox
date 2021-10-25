#ifndef I2C_MLINER_H
#define I2C_MLINER_H
#include <stdint.h>

#define BUFFER_SIZE_I2C_MLINER EXACTOLINK_MESSAGE_SIZE

extern void bind_TX_thread_I2C_MLINER(int (*run)(struct lthread *));
extern void bind_RX_thread_I2C_MLINER(int (*run)(struct lthread *));
#endif //
