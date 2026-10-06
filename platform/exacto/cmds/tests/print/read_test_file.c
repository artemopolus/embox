#include "writer_smpl.h"

#include <stdio.h>

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

#define MAX_PRINT 64 
#define HEAD_BYTES 16 
#define TAIL_BYTES 16

static const char hex[] = "0123456789ABCDEF";

void cv_Uint8_Int16(uint8_t * src, int16_t * dst)
{
    int16_t first = (int16_t) src[1];
    *dst = (first << 8) + (int16_t)src[0];
}

// =================
static void print_values(const uint8_t *data, uint16_t len, uint16_t rows)
{
    // Первые 6 байт пропускаем
    const uint16_t offset = 6;

    // 1 строка = 6 байт = 3 значения int16_t
    const uint16_t packet_size = 6;

    // Проверяем, что после первых 6 байт есть данные
    if (len <= offset) {
        return;
    }

    // Сколько полных строк реально есть в буфере
    uint16_t available_rows = (len - offset) / packet_size;

    // Не выводим больше, чем есть данных
    if (rows > available_rows) {
        rows = available_rows;
    }

    printf(" #      Value 1      Value 2      Value 3\n");
    printf("------------------------------------------\n");

    for (uint16_t i = 0; i < rows; i++) {

        // Начало текущей пачки
        const uint8_t *src = &data[offset + i * packet_size];

        int16_t value1;
        int16_t value2;
        int16_t value3;

        cv_Uint8_Int16(&src[0], &value1);
        cv_Uint8_Int16(&src[2], &value2);
        cv_Uint8_Int16(&src[4], &value3);

        printf("%3u    %10d    %10d    %10d\n",
               i + 1,
               value1,
               value2,
               value3);
    }
}
// =================

static void print_hex(const uint8_t *data, uint16_t len)
{
    uint16_t end_head = len;
    uint16_t start_tail = len;

    if (len > MAX_PRINT) {
        end_head = HEAD_BYTES;
        start_tail = len - TAIL_BYTES;
    }

    // Верх
    for (uint16_t i = 0; i < end_head; i++) {
        if ((i % 16) == 0) {
            putchar('\n');

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

    // Пропущенная середина
    if (len > MAX_PRINT) {
        printf("\n....\n");

        // Низ
        for (uint16_t i = start_tail; i < len; i++) {
            if ((i % 16) == 0) {
                putchar('\n');

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
    }

    putchar('\n');
}


// #include <stdio.h>
// #include <stdint.h>
// #include <stddef.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <errno.h>
// #include <string.h>

static int readOneFile(const char *filename, int print_mode)
{
    uint8_t read_buffer[128];
    ssize_t bytes_read;
    int file = open(filename, O_RDONLY);

    if (file < 0)
    {
        printf("Can't open file %s, errno=%d (%s)\n",
               filename, errno, strerror(errno));
        return 1;
    }

    printf("\nReading file: %s\n", filename);

    while (1)
    {
        bytes_read = read(file, read_buffer, sizeof(read_buffer));

        if (bytes_read < 0)
        {
            printf("Error reading file %s, errno=%d (%s)\n",
                   filename, errno, strerror(errno));
            close(file);
            return 1;
        }

        if (bytes_read == 0)
        {
            break;
        }

        if (print_mode == 1)
        {
            print_hex(read_buffer, (size_t)bytes_read);
        }
        else if (print_mode == 0)
        {
            print_values(read_buffer, (size_t)bytes_read, 6);
        }
        else 
        {
        }
    }

    close(file);

    printf("\nEnd of file: %s\n", filename);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <filename> <mode>\n", argv[0]);
        printf("  mode 0 - print_values\n");
        printf("  mode 1 - print_hex\n");
        return 1;
    }

    int mode = atoi(argv[2]);

    // if (mode != 0 && mode != 1)
    // {
    //     printf("Error: mode must be 0 or 1\n");
    //     return 1;
    // }

    return readOneFile(argv[1], mode);
}