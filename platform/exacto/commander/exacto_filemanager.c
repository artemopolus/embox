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

#define EX_FM_PATH_TO_FILE_PT 17
#define EX_FM_PATH_TO_LOG_PT 12
#define EX_FM_LEN 12


//                            0123456789012345678901234567890123456
char ExFmPathToFIle[] =   "/mnt/DATA/sessionYYMMDDHHMMSS.txt";
char ExFmPathToLog[] =    "/mnt/LOG/logYYMMDDHHMMSS.txt";
char ExFmSessionName[] = "YYMMDDHHMMSS";

uint8_t ex_saveToFile(uint8_t * data, uint16_t datalen)
{
    return 0;
}
uint8_t ex_saveToLog(uint8_t * data, uint16_t datalen)
{
    return 0;
}


EMBOX_UNIT_INIT(initExactoFileManager);
static int initExactoFileManager(void)
{
    DIR* dir = opendir("/mnt/LOG");
    if (!dir)
    {
        mkdir("/mnt/LOG", 0777);
        dir =  opendir("/mnt/LOG");
        if (!dir)
         return 1;
    }
    for (int i = 0; i < 999999999; i++)
    {
        for (int y = 0; y < 12; y++)
        {
            ExFmSessionName[y] = '\0';
            ExFmPathToLog[y + EX_FM_PATH_TO_LOG_PT] = '0';
        }
        itoa(i, ExFmSessionName, 10);
        int len = 12;
        for (int y = 0; y < 12; y++)
        {
            if (ExFmSessionName[y] == '\0')
            {
                len = y;
                break;
            }
        }
        for (int y = 0; y < len; y++)
        {
            ExFmPathToLog[EX_FM_PATH_TO_LOG_PT + 12 - y] = ExFmSessionName[len - y];
        }
        if (access(ExFmPathToLog, F_OK) != 0)
        {
            break;
        }
    }
    return 0;
}
