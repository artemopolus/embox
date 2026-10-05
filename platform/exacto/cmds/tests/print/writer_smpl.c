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

#include <stdint.h>
#include <errno.h>
#include <limits.h>


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

#define WRITE_FILE_BUFFER_SZ 1024


typedef struct{
    int File;
    uint8_t status;
    uint8_t write_flag;
    uint16_t write_event_cnt;
    uint16_t file_write_dones;
    uint32_t write_cnt;
    char filename[64];
    uint8_t buffer[WRITE_FILE_BUFFER_SZ];
    ExactoBufferUint8Type Store;
}ExactoFile;

ExactoFile AccDataFile = {
    .status = 0,
    .write_flag = 1,
    .write_event_cnt = 0,
    .write_cnt = 0,
    .file_write_dones = 0,
    .filename = "none",
};
ExactoFile GyrDataFile = {
    .status = 0,
    .write_flag = 1,
    .write_event_cnt = 0,
    .write_cnt = 0,
    .file_write_dones = 0,
    .filename = "none",
};
	
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

char target_prefix[64];

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

/*
 * prefix     - префикс имени, например "/mnt/data/acc"
 * digits     - количество цифр номера
 * result     - буфер для полного имени файла
 * result_size - размер буфера result
 *
 * Возвращает:
 *   0 - номер найден
 *   1 - ошибка
 */
uint8_t get_file_name(
    const char *prefix,
    unsigned int digits,
    char *result,
    size_t result_size)
{
    if (prefix == NULL || result == NULL ||
        digits == 0 || digits > 9 || result_size == 0)
    {
        return 1;
    }

    size_t prefix_len = strlen(prefix);

    if (prefix_len + digits + 1 > result_size)
    {
        return 1;
    }

    char number[10];

    const char *suffix = "_acc.bin";
    size_t suffix_len = strlen(suffix);

    for (unsigned int i = 0; i < 1000000000U; ++i)
    {
        int written = snprintf(number, sizeof(number),
                               "%0*u", (int)digits, i);

        if (written < 0 || (unsigned int)written != digits)
        {
            return 1;
        }

        if (prefix_len + digits + suffix_len + 1 > result_size)
        {
            return 1;
        }

        memcpy(result, prefix, prefix_len);
        memcpy(result + prefix_len, number, digits + 1);
        memcpy(result + prefix_len + digits, suffix, suffix_len + 1);

        FILE *file = fopen(result, "r");

        if (file == NULL)
        {
            if (errno == ENOENT)
            {
                result[prefix_len + digits] = '\0';
                return 0;
            }

            return 1;
        }

        fclose(file);
    }

    return 1;
}

int get_filenames()
{
    DIR* dir = opendir("/mnt/data");
    if (!dir)
    {
        printf("Dir is created\n");
        mkdir("/mnt/data", 0777);
        dir =  opendir("/mnt/data");
        if (!dir)
        {
            printf("Dir mnt/data not created");
            return 1;
        }
    }
    else
    {
        printf("Log dir is found\n");
    }
    closedir(dir);


    if (get_file_name("/mnt/data/sns", 6, target_prefix, sizeof(target_prefix)) == 0)
    {
        snprintf(AccDataFile.filename, sizeof(AccDataFile.filename),"%s_acc.bin", target_prefix);
        snprintf(GyrDataFile.filename, sizeof(GyrDataFile.filename),"%s_gyr.bin", target_prefix);
        printf("Acc filename:%s\n", AccDataFile.filename);
        printf("Gyr filename:%s\n", GyrDataFile.filename);
        AccDataFile.status = 1;
        GyrDataFile.status = 1;
    }
    else
    {
        return 2;
    }
    return 0;
}
void close_one_file( ExactoFile * trg)
{
    if (trg->status == 0 )
        return;
    close(trg->File);
    trg->status = 0;
    trg->write_cnt = 0;
    trg->write_event_cnt = 0;
    trg->file_write_dones = 0;
}
void close_files()
{
    close_one_file( & AccDataFile );
    close_one_file( & GyrDataFile );
    printf("Close files\n");
}
int open_one_file(ExactoFile * trg)
{
    printf("Try to open %s\n", trg->filename);
    if (trg->status == 1)
    {
        trg->File = open(trg->filename,O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (0 > trg->File)
        {
            printf("Can't open Data file: %s\n", trg->filename);
            return 1;
        }
        printf("Open file %s\n", trg->filename);
        trg->status = 2;
        return 0;
    }
    printf("Bad status\n");
    return 1;
}
int open_files()
{
    if ((!open_one_file(&AccDataFile)) && (!open_one_file(&GyrDataFile)))
    {
        return 0;
    }
    printf("error open files\n");
    return 1;
}

void readOneFile( ExactoFile * trg)
{
    uint8_t read_buffer[128];
    ssize_t bytes_read;
    trg->File = open(trg->filename, O_RDONLY);
    if (trg->File < 0) {
        printf("Can't open file %s, errno=%d\n", trg->filename, errno);
        return;
    }

    printf("\nReading file: %s\n", trg->filename);

    while (1) {
        bytes_read = read(trg->File, read_buffer, sizeof(read_buffer));

        if (bytes_read < 0) {
            printf("Error reading file %s, errno=%d\n",
                   trg->filename, errno);
            break;
        }

        if (bytes_read == 0) {
            break;
        }

        print_hex(read_buffer, (uint16_t)bytes_read);
    }

    close(trg->File);
    printf("\nEnd of file: %s\n", trg->filename);
}

void readFileAndPrintHex(const char *filename) 
{

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

    readOneFile( & AccDataFile );

    readOneFile( & GyrDataFile );
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
void writeBuffer( ExactoFile * trg)
{
    if (getlen_exbu8(&(trg->Store)))
    {
        trg->write_flag = 0;
        uint16_t copy = grbfstPack_exbu8(&(trg->Store), trg->buffer, 128);
        if (write( trg->File, trg->buffer, copy ))
        {

        }
        else
        {
            trg->file_write_dones ++;
        }
        trg->write_flag = 1;
    }
}
void write_to_buffers( )
{
    writeBuffer( & AccDataFile);
    writeBuffer( & GyrDataFile);
}

static void *runMainBasicThread(void *arg) {
    
	printf("Start thread with SD writing\n");

    while(BBBFlag == 0)
        usleep(1100000);
    BBBFlag = 0;
    if (get_filenames())
    {
        printf("Error: get file names\n");
        return NULL;
    }
    if (open_files())
    {
        printf("Error: open files\n");
        return NULL;
    }
    else
    {
        printf("Files are opened\n");
    }
    // Pt = open("/mnt/test.txt",O_CREAT | O_WRONLY | O_TRUNC, 0666);
	// if (0 > Pt)
	// {
    //   	printf("Can't open Data file\n");
	// 	return NULL;
	// }
    FileOpen = 1;
    uint8_t Header[] = {7, 7, 7, 7, 7, 7};
    uint8_t Ender[] = {4, 4, 4, 4, 4, 4};
	// else
    //   printf("Data file is opened\n");

    addDataToFile(0, Header, 6);
    addDataToFile(1, Header, 6);


    // addDataToWrite( Header, 6);
    // writeBufferDataToFile();
    write_to_buffers();
    AccDataFile.write_flag = 1;
    GyrDataFile.write_flag = 1;
	// Print2SDFlag = 1;
    BBBFlag = 1;
    // printf("Test end\n");
    // close_files();
    // FileOpen = 0;
    // return NULL;

	while (BBBFlag)
	{
		while (AccDataFile.write_flag == 0 && GyrDataFile.write_flag == 0)
		{
		}

        // writeBufferDataToFile();
        write_to_buffers();

		usleep(10000);
	}
	
    addDataToFile(0, Ender, 6);
    addDataToFile(1, Ender, 6);

    // addDataToWrite( Ender, 6);
    // writeBufferDataToFile();
    write_to_buffers();
    close_files();
    // close(Pt);
    FileOpen = 0;
    
	return NULL;
}
void openFileSD()
{
    BBBFlag = 1;
}
uint8_t isReadyToWrite()
{
	// return Print2SDFlag;
    return AccDataFile.write_flag && GyrDataFile.write_flag;
}
void blockWrite()
{
    AccDataFile.write_flag = 0;
    GyrDataFile.write_flag = 0;
}
void unBlockWrite()
{
    AccDataFile.write_flag = 1;
    GyrDataFile.write_flag = 1;
}
void addDataToFile(uint8_t file_id, uint8_t *data, uint16_t datalen)
{
    ExactoFile * trg;
    if (file_id == 0)
    {
        trg = & AccDataFile;
    }
    else if (file_id == 1)
    {
        trg = & GyrDataFile;
    }
    else
    {
        return;
    }
    trg->write_flag = 0;
    trg->write_cnt+= datalen;
    trg->write_event_cnt ++;
    pshsftPack_exbu8(&(trg->Store), data, datalen);
    trg->write_flag = 1;
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
	printf("Thread started: %d\n"
            "ACC events: %d\n"
            "Data added len: %d\n"
            "GYR events: %d\n"
            "Data added len: %d\n"
        , BBBFlag, AccDataFile.write_event_cnt, AccDataFile.write_cnt,
        GyrDataFile.write_event_cnt, GyrDataFile.write_cnt
    );
}
EMBOX_UNIT_INIT(initTestSmplMod);
static int initTestSmplMod()
{
	Print2SDFlag = 1;
	ReaderAddCounter = 0;
	ReaderDataLen = 0;

    unBlockWrite();

    BBBFlag = 0;
    FileOpen = 0;
	setini_exbu8(&ReaderStore);
    setini_exbu8(&(AccDataFile.Store));
    setini_exbu8(&(GyrDataFile.Store));
	MainBasicThread = thread_create(THREAD_FLAG_DETACHED |THREAD_FLAG_SUSPENDED, runMainBasicThread, NULL);
    thread_launch(MainBasicThread);
	return 0;
}
