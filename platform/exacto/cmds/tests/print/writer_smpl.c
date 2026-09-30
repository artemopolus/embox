#include <stdio.h>
#include <kernel/sched.h>
#include <kernel/sched/waitq.h>
#include <kernel/sched/schedee_priority.h>
#include <kernel/lthread/lthread.h>
#include <kernel/thread.h>
#include <kernel/time/ktime.h>
#include <kernel/sched/sync/mutex.h>
#include <kernel/lthread/sync/mutex.h>
#include <kernel/thread/sync/mutex.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <embox/unit.h>

#include "writer_smpl.h"

#include "operator/exacto_buffer.h"
	
ExactoBufferUint8Type ReaderStore;

static uint8_t buffer[1048];
static uint8_t Print2SDFlag;
static int PrintRes;
static uint8_t BBBFlag = 0;
static int Pt;

static struct thread *MainBasicThread;

uint16_t ReaderAddCounter;
uint16_t ReaderDataLen;
uint16_t FileDataLen;
uint8_t FileOpen;

static const char hex[] = "0123456789ABCDEF";

static void print_hex(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {

        if ((i % 16) == 0) {
            putchar('\n');

            // индекс элемента
            putchar(hex[(i >> 12) & 0x0F]);
            putchar(hex[(i >> 8)  & 0x0F]);
            putchar(hex[(i >> 4)  & 0x0F]);
            putchar(hex[i & 0x0F]);

            putchar(':');
            putchar(' ');
        }

        putchar(hex[data[i] >> 4]);
        putchar(hex[data[i] & 0x0F]);
        putchar(' ');
    }

    putchar('\n');
}

void readFileAndPrintHex(const char *filename) 
{
    int fd;
    uint8_t read_buffer[128];
    ssize_t bytes_read;

    /*
     * Если файл всё ещё открыт потоком записи,
     * останавливаем цикл записи.
     */
    if (BBBFlag == 1) {
        BBBFlag = 0;

        /*
         * Поток записи после выхода из while(BBBFlag)
         * выполнит close(Pt). Даём ему завершить эту операцию.
         */
        while(FileOpen)
            sleep(1);
    }

    fd = open(filename, O_RDONLY);

    if (fd < 0) {
        printf("Can't open file %s, errno=%d\n", filename, errno);
        return;
    }

    printf("\nReading file: %s\n", filename);

    while (1) {
        bytes_read = read(fd, read_buffer, sizeof(read_buffer));

        if (bytes_read < 0) {
            printf("Error reading file %s, errno=%d\n",
                   filename, errno);
            break;
        }

        if (bytes_read == 0) {
            break;
        }

        print_hex(read_buffer, (uint16_t)bytes_read);
    }

    close(fd);

    printf("\nEnd of file: %s\n", filename);
}

void readTestFile()
{
	readFileAndPrintHex("/mnt/test.txt");
}
void writeBufferDataToFile()
{
    if (getlen_exbu8(&ReaderStore))
    {
        printf("Write data to file\n");
        Print2SDFlag = 0;
        uint16_t copy = grbfstPack_exbu8(&ReaderStore, buffer, 128);
        PrintRes = write (Pt, buffer, copy);
        Print2SDFlag = 1;
    }
}

static void *runMainBasicThread(void *arg) {
    
	printf("Start thread\n");

    while(BBBFlag == 0)
        usleep(1100000);
    BBBFlag = 0;
    Pt = open("/mnt/test.txt",O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (0 > Pt)
	{
      	printf("Can't open Data file\n");
		return NULL;
	}
    FileOpen = 1;
    uint8_t Header[] = {7, 7, 7, 7, 7, 7};
    uint8_t Ender[] = {4, 4, 4, 4, 4, 4};
	// else
    //   printf("Data file is opened\n");
    addDataToWrite( Header, 6);
    writeBufferDataToFile();
	Print2SDFlag = 1;
    BBBFlag = 1;
	while (BBBFlag)
	{
		while (Print2SDFlag == 0)
		{
		}

        writeBufferDataToFile();

		usleep(10000);
	}
	
    addDataToWrite( Ender, 6);
    writeBufferDataToFile();
    close(Pt);
    FileOpen = 0;
    
	return NULL;
}
void openFileSD()
{
    BBBFlag = 1;
}
uint8_t isReadyToWrite()
{
	return Print2SDFlag;
}
void addDataToWrite( uint8_t * data, uint16_t datalen)
{
    // if (BBBFlag == 0)
        // return;
	Print2SDFlag = 0;
	ReaderAddCounter ++;
	ReaderDataLen += datalen;
	pshsftPack_exbu8(&ReaderStore, data, datalen);
	Print2SDFlag = 1;
}
void printReaderData()
{
	printf("Thread started: %d\nAdd Data Events Count: %d\nData added len: %d\n", BBBFlag, ReaderAddCounter, ReaderDataLen);
}
EMBOX_UNIT_INIT(initTestSmplMod);
static int initTestSmplMod()
{
	Print2SDFlag = 1;
    BBBFlag = 0;
	ReaderAddCounter = 0;
	ReaderDataLen = 0;
    FileOpen = 0;
	setini_exbu8(&ReaderStore);
	MainBasicThread = thread_create(THREAD_FLAG_DETACHED |THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);
    thread_launch(MainBasicThread);
	return 0;
}
