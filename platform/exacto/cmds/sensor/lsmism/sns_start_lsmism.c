#include "sensors/sns_lsmism.h"



int main(int argc, char *argv[]) 
{
    int type = 0;

    if (argc > 1)
    {
        if (*argv[1] == '0')
            type = 0;
        else if (*argv[1] == '1')
            type = 1;     
        else if (*argv[1] == '2')
            type = 2;     
        else if (*argv[1] == '3')
            type = 3;     
        else if (*argv[1] == '4')
            type = 4;     
    }

    printf("Start lsm303ah ism330dlc module: %d\n", type);
    exSnsStart(type);
    printf("Done\n");
    return 0;
}
