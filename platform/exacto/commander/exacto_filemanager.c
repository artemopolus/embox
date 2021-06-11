#include "exacto_filemanager.h"
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
#include <kernel/printk.h>

#define EX_FM_PATH_TO_FILE_PT 17
#define EX_FM_PATH_TO_LOG_PT 12
#define EX_FM_LEN 12


//                         0123456789012345678901234567890123456
char ExFm_File_Path[] =   "/mnt/DATA/sessionYYMMDDHHMMSS.txt";
char ExFm_Log_Path[] =    "/mnt/LOG/logYYMMDDHHMMSS.txt";
char ExFm_Session_Name[] = "YYMMDDHHMMSS";

FILE * ExFm_Log_Pointer;
int ExFm_File_Pointer;

uint8_t     ExFm_Data_Buffer[EXACTO_BUFFER_UINT8_SZ] = {0};
uint16_t    ExFm_Data_length = 0;
uint16_t    ExFm_Data_lengthmax = 8192;
uint32_t    ExFm_Data_pulledcnt = 0;

static uint32_t EFM_PushToBuffer_BasicCnt = 0;
static uint32_t EFM_PushToBuffer_TmpCnt = 0;
static uint32_t EFM_BufferToSD_BasicCnt = 0;

uint8_t ex_writeToLogChar(char * info)
{
    if (ExFm_Log_Pointer == NULL)
        return 1;
    fprintf(ExFm_Log_Pointer, info);
    return 0;
}
uint8_t ex_saveExBufToFile( ExactoBufferUint8Type * buffer )
{
    uint8_t value;
    EFM_PushToBuffer_BasicCnt++;
    EFM_PushToBuffer_TmpCnt++;
    ExFm_Data_length = 0;
    while(grbfst_exbu8(buffer,&value))
    {
        ExFm_Data_Buffer[ExFm_Data_length++] = value;
        if (ExFm_Data_length >= ExFm_Data_lengthmax)
            break;
    }
    return 0;
}
uint8_t ex_pshExBufToSD(  )
{
    if (ExFm_Data_length % EXACTOLINK_SD_FRAME_SIZE)
    {
        //почему то размеры буфера не совпадают
        printk(".");
    }
    EFM_PushToBuffer_TmpCnt = 0;
    // uint8_t index = 0;
    // while(512*index < ExFm_Data_length)
    // {
    int res = pwrite (ExFm_File_Pointer, ExFm_Data_Buffer, ExFm_Data_length,0);
	    if (res<=0) {
           return 1;
        }
    // }
    if (res != ExFm_Data_length)
        return 1;
    EFM_BufferToSD_BasicCnt++;
    ExFm_Data_pulledcnt += ExFm_Data_length;
    ExFm_Data_length = 0;
    return 0;
}
uint8_t ex_saveToFile(uint8_t * data, uint16_t datalen)
{
    // for (uint16_t i = 0; i < datalen; i++)
    // {
    //     fprintf(ExFm_Log_Pointer, "%d", data[i]);
    // }
    // fprintf(ExFm_Log_Pointer, "\n");
    
	if (write (ExFm_File_Pointer, data, datalen)<=0) {
        return 1;
    }
    return 0;
}
uint8_t ex_saveToLog(uint8_t * data, uint16_t datalen)
{
    return 0;
}


// EMBOX_UNIT_INIT(initExactoFileManager);
uint8_t initExactoFileManager(void)
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
    // p_file = fopen(ExFmPathToLog, "w+");
    // p_file = fopen("/mnt/f9.txt", "w+");
    if (ExFm_Log_Pointer != NULL)
    {
        printf("File is opened\n");
        fprintf(ExFm_Log_Pointer, "Session is started\n");
    //     fprintf(p_file, "%d", 12567);
    //     // fprintf(p_file, "y");
    //     // fprintf(p_file, "y");
        ExFm_File_Pointer = creat(ExFm_File_Path,0);
        if( 0 > ExFm_File_Pointer)
        {
            printf("Can't open Data file\n");
            ex_writeToLogChar("Can't open Data file\n");
            return 1;
        }
        else
        {
            printf("Data file is opened\n");
            ex_writeToLogChar("Data file is opened\n");
        }
    }
    else
    {
        printf("Can't open file\n");
        return 1;
    }
return 0;
}
