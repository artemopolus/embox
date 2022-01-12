#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
  
#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/thread.h>
#include <kernel/thread/sync/mutex.h>
#include <kernel/thread/sync/cond.h>
#include <kernel/lthread/lthread.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/task.h>
#include <kernel/time/ktime.h>
#include <kernel/printk.h>

#include "tim/tim.h"
  
#include "commander/exacto_data_storage.h"
#include "commander/exacto_filemanager.h"
// #include "sensors/exacto_datatools.h"

#include "spi/spi_mliner.h"
#include "gpio/gpio_spi.h"
#include "gpio/gpio_led.h"

int main(int argc, char *argv[]) {
    return 0;
}
