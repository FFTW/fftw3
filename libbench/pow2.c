#include "bench.h"

int power_of_two(unsigned int n)
{
     return (((n) > 0) && (((n) & ((n) - 1)) == 0));
}
