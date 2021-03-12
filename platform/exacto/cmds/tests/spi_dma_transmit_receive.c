#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
  
#include <stdint.h>
#include "commander/exacto_data_storage.h"

thread_control_t MainThread;

uint8_t MarkerThread = 0;
uint8_t DataToBuffer[] = {0, 7, 2, 10, 1};
struct lthread UpdateDataToBufferThread;
struct lthread SendDataThread;

struct lthread PrintThread;
struct lthread MarkerCheckerThread;
static int updateDataToBufferThreadRun(struct lthread * self)
{
    setDataToExactoDataStorage(DataToBuffer, 5); 
    return 0;
}
static int sendDataThreadRun(struct lthread * self)
{
    transmitExactoDataStorage();
    return 0;
}
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
    printf("Set thread for data uploading\n");
    lthread_init(&UpdateDataToBufferThread, updateDataToBufferThreadRun);
    printf("Set thread for data sending\n");
    lthread_init(&SendDataThread, sendDataThreadRun);
    printf("Upload data to buffer\n");
    lthread_launch(&UpdateDataToBufferThread);
    printf("Send data\n");
    lthread_launch(&SendDataThread);
    printf("Run cycle for checking\n");
    uint8_t pt = 0;
    const uint8_t pt_max = 50;
    while (!MarkerThread)
    {
        checkExactoDataStorage(&MainThread);
        lthread_launch(&MarkerCheckerThread);
		usleep(100000);
        
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
