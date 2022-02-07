#include "sensors/sns_lsmism.h"



int main(int argc, char *argv[]) 
{
    int type = 0;

    if (argc > 1)
    {
        if (*argv[1] == '0')
            type = (uint8_t)EXACTOLINK_SNS_XL_0100_XLGR_0100;
        else if (*argv[1] == '1')
            type = (uint8_t)EXACTOLINK_SNS_XL_0200_XLGR_0100;     
        else if (*argv[1] == '2')
            type = (uint8_t)EXACTOLINK_SNS_XL_0400_XLGR_0100;     
        else if (*argv[1] == '3')
            type = (uint8_t)EXACTOLINK_SNS_XL_0200_XLGR_0200;     
        else if (*argv[1] == '4')
            type = (uint8_t)EXACTOLINK_SNS_XL_0400_XLGR_0400;     
    }

    printf("Start lsm303ah ism330dlc module: %d\n", type);
    exSnsStart(type);
    printf("Done\n");
    return 0;
}
