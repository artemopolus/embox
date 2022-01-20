#include "sensors/sns_lsmism.h"



int main(int argc, char *argv[]) 
{

    printf("Start lsm303ah ism330dlc module\n");
    exSnsStart();
    printf("Done\n");
    return 0;
}
