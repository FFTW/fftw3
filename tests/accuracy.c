#include <math.h>
#include <stdlib.h>
#include "ifftw.h"
#include "dft.h"
#include "rdft.h"

extern void mfft(int n, R *a, int sign);

int main(int argc, char *argv[])
{
     int n = 1024;
     planner *plnr;
     problem *prblm;
     plan *pln;
     R *a, *a0;
     int i;
     R e1, e2, einf;

     if (argc > 1)
	  n = atoi(argv[1]);

     plnr = FFTW(mkplanner_score)(ESTIMATE);
     FFTW(dft_conf_standard)(plnr);
     a = fftw_malloc(2 * n * sizeof(R), OTHER);
     a0 = fftw_malloc(2 * n * sizeof(R), OTHER);
     prblm =
	  FFTW(mkproblem_dft_d)(
	       FFTW(mktensor_1d)(n, 2, 2), FFTW(mktensor)(0), a, a+1, a, a+1);
     pln = plnr->adt->mkplan(plnr, prblm);
     AWAKE(pln, 1);
     for (i = 0; i < 2 * n; ++i) 
	  a0[i] = a[i] = drand48();
     pln->adt->solve(pln, prblm);
     mfft(n, a0, -1);
     
     e1 = e2 = einf = 0.0;
     for (i = 0; i < 2 * n; ++i) {
	  double d = a[i] - a0[i];
	  if (d < 0) d = -d;
	  e1 += d;
	  e2 += d * d;
	  if (d > einf) einf = d;
     }
     e1 /= 2 * n;
     e2 = sqrt(e2 / (2 * n));

     printf("e1: %g e2: %g einf: %g\n", e1, e2, einf);
}
