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

#include "logger.h"

#include <embox/unit.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

#include <fcntl.h>

#define EX_FM_PATH_TO_FILE_PT 17
#define EX_FM_PATH_TO_LOG_PT 12
#define EX_FM_LEN 12

#define EFM_BUFF_SIZE 8192
#define EFM_MESS_THRE 1024

static char ExFm_File_Path[] =   "/mnt/DATA/sessionYYMMDDHHMMSS.txt";
static char ExFm_Log_Path[] =    "/mnt/LOG/logYYMMDDHHMMSS.txt";
static char ExFm_Session_Name[] = "YYMMDDHHMMSS";
static int  ExFm_Session_Num  = 0;
static FILE * ExFm_Log_Pointer;
static int ExFm_File_Pointer;


typedef struct exlg_dev{
	uint8_t     buffer[EFM_BUFF_SIZE];
	uint16_t    length;
	uint16_t    lengthmax;
	uint32_t    pulledcnt;
	uint32_t    pulledmax;
}exlg_dev_t;


static exlg_dev_t ExLgDev = {
	.buffer = {0},
	.length = 0,
	.lengthmax = EFM_BUFF_SIZE,
	.pulledcnt = 0,
	.pulledmax = 5000,
};

int exlg_updateLogger()
{
	if (ExLgDev.length < EFM_MESS_THRE)
		return 1;
	if (ExLgDev.length > EFM_BUFF_SIZE)
	{
		//почему то размеры буфера не совпадают
		return 2;
	}
	int res;
   res = write (ExFm_File_Pointer, ExLgDev.buffer, ExLgDev.length);
	if (res<=0) 
	{
		return 3;
	}
	if (res != ExLgDev.length)
		return 4;
	if (ExLgDev.pulledcnt > ExLgDev.pulledmax)
	{
	}
	else
	{
		ExLgDev.pulledcnt += ExLgDev.length;
	}
	ExLgDev.length = 0;
   return 0;
}

uint8_t exlg_uploadLogger(uint8_t * data, uint16_t datalen)
{
	if (datalen > ExLgDev.lengthmax - ExLgDev.length)
		return 1;
	memcpy(&ExLgDev.buffer[ExLgDev.length], data, datalen);
	ExLgDev.length += datalen;
	return 0;
}

int exlg_initLogger()
{
	DIR* dir = opendir("/mnt/LOG");
	if (!dir)
	{
		printf("Log dir is created\n");
		mkdir("/mnt/LOG", 0777);
		dir =  opendir("/mnt/LOG");
		if (!dir)
		return 1;
	}
	else
	{
		printf("Log dir is found\n");
	}
	closedir(dir);

	FILE * pointer;
	for (int i = 0; i < 999999999; i++)
	{
		for (int y = 0; y < 12; y++)
		{
			ExFm_Session_Name[y] = '\0';
			ExFm_Log_Path[y + EX_FM_PATH_TO_LOG_PT] = '0';
			ExFm_File_Path[y + EX_FM_PATH_TO_FILE_PT] = '0';
		}
		itoa(i, ExFm_Session_Name, 10);
		int len = 12;
		for (int y = 0; y < 12; y++)
		{
			if (ExFm_Session_Name[y] == '\0')
			{
					len = y;
					break;
			}
		}
		for (int y = 0; y < len; y++)
		{
			char value = ExFm_Session_Name[len - y - 1 ];
			ExFm_Log_Path[EX_FM_PATH_TO_LOG_PT + 11 - y] = value;
			ExFm_File_Path[EX_FM_PATH_TO_FILE_PT + 11 - y] = value;
		}
		ExFm_Session_Num = i;
		pointer = fopen(ExFm_Log_Path, "r");
		if (pointer != NULL)
		{
			printf("\033[A\33[2K\r");
			printf("Find file %s\n", ExFm_Log_Path);
			fclose(pointer);
		}
		else
		{
			printf("Write to log file: %s\n", ExFm_Log_Path);
			fclose(pointer);
			break;
		}
	}
	printf("Try to open file\n");
	ExFm_Log_Pointer = fopen(ExFm_Log_Path, "w");
	if (ExFm_Log_Pointer != NULL)
	{
		printf("File is opened\n");
		fprintf(ExFm_Log_Pointer, "Session is started\n");
		ExFm_File_Pointer = open(ExFm_File_Path,O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0666);
		if( 0 > ExFm_File_Pointer)
		{
			printf("Can't open Data file\n");
			// ex_writeToLogChar("Can't open Data file\n");//TODO: log to session
			return 1;
		}
		else
		{
			printf("Data file is opened\n");
			// ex_writeToLogChar("Data file is opened\n"); //TODO: log to session
		}
	}
	else
	{
		printf("Can't open file\n");
		return 1;
   }
	return 0;
}
