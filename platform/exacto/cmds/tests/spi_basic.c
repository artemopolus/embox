#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"

thread_control_t MainThread;

uint8_t MarkerThread = 0;

struct lthread PrintThread;
struct lthread MarkerCheckerThread;
/**
 * @brief just indicator
 * 
 * @param self thread 
 * @return int 
 */
static int printThreadRun(struct lthread * self)
{
    printf("Test spi done\n");
    return 0;
}
/**
 * @brief check marker form data storage
 * 
 * @param self light thread
 * @return int 
 */
static int checkMarkerThreadRun(struct lthread * self)
{
    if (MainThread.result == THR_CTRL_OK)
    {
        MarkerThread = 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("Set control marker thread\n");
    lthread_init(&MarkerCheckerThread, checkMarkerThreadRun);
    printf("Set main thread for data working\n");
    initThreadExactoDataStorage(&MainThread);
    printf("Set printf thread\n");
    lthread_init(&PrintThread, printThreadRun);
    printf("Set thread for data sending\n");

    printf("Run cycle for checking\n");
    uint8_t pt = 0;
    const uint8_t pt_max = 5;
    while (!MarkerThread)
    {
        checkExactoDataStorage(&MainThread);
        lthread_launch(&MarkerCheckerThread);
        sleep(1);
        if (pt < pt_max)
        {
            pt++;
            printf(".");
        }
        else{
            pt = 0;
            printf("\33[2K\r");
        }
    }
    lthread_launch(&PrintThread);

    printf("Programm reach end\n");
    return 0;
}
