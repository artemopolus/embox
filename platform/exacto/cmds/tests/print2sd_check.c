
#include "tests/print2sd.h"
extern uint8_t Print2SDFlag;

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

    printf("\nDone\n");

	return 0;
}
