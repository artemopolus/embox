
#include "smpl_mod.h"

int main(int argc, char *argv[]) 
{
    printf("Starting... check\n");
    if 	(Print2SDFlag)
    {
        printf("Yes");
    }
    else
    {
        printf("No");
    }

    printf("\nDone: %d\n", PrintRes);
    Print2SDFlag = 0;

	return 0;
}
