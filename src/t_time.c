#include "t_time.h"

unsigned long long
get_time(void)
{
    __asm ("RDTSC");
}


