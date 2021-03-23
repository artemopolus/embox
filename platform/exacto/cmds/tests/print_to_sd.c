#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {

    printf("I try to read data from sd\n");

    FILE * p_file;
    uint8_t buffer[20] = {0};

    p_file = fopen("/mnt/file.txt", "r");

    if (p_file != NULL)
    {
        fscanf(p_file, "%s", buffer);
        for (uint8_t i = 0; i < 60; i++)
            printf("%c",buffer[i]);
        fclose(p_file);
    }
    else
    {
        printf("Can't open file\n");
    }

    return 0;
}
