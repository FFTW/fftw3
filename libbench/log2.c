#include "bench.h"

int log_2(int n)
{
     int m = 0;

     while (n > 1) {
	  ++m;
	  n >>= 1;
     }

     return m;
}
