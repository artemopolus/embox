#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static void sending()
{

}
static void init()
{

}
int main(int argc, char *argv[]) 
{
    int index_max = -1, var_cnt = 2;
	int opt;
    extern char * optarg;
	while((opt = getopt(argc, argv, "M:f:h")) != -1)
    {
		switch (opt) {
		case 'M':
            if (optarg) 
                index_max = atoi(optarg);
			break;
		case 'f':
            if (optarg) 
                var_cnt = atoi(optarg);
            break;
		case 'h':
            printf(
                "Usage:\ntest_read -- infinity number of readings\ntest_read -M [number of reading iterations, default = inf]\n-f [number of device to search, default = 2]");
           return 0; 
        }
	}
    init();
    sending();
    return 1;
}