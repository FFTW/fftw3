#include <math.h>
#include <stdlib.h>
#include "ifftw.h"
#include "dft.h"
#include "rdft.h"

extern void mfft(int n, R *a, int sign);

int main(int argc, char *argv[])
{
     int nmin = 1024, nmax, n;
     planner *plnr;
     problem *prblm;
     plan *pln;
     R *a, *a0;
     int i;
     R e1, e2, einf, re1, re2, reinf, mag;

     srand48(1);

     if (argc > 1)
	  nmin = atoi(argv[1]);
     if (argc > 2)
	  nmax = atoi(argv[2]);
     else
	  nmax = nmin;

     if (argc > 3)
	  srand48(atoi(argv[3]));

     plnr = FFTW(mkplanner_score)(ESTIMATE);
     FFTW(dft_conf_standard)(plnr);
     a = fftw_malloc(2 * nmax * sizeof(R), OTHER);
     a0 = fftw_malloc(2 * nmax * sizeof(R), OTHER);

     for (n = nmin; n <= nmax; n = 2 * n) {
	  prblm =
	       FFTW(mkproblem_dft_d)(
		    FFTW(mktensor_1d)(n, 2, 2), FFTW(mktensor)(0), a, a+1, a, a+1);
	  pln = plnr->adt->mkplan(plnr, prblm);
	  AWAKE(pln, 1);
	  mag = 0.0;
	  for (i = 0; i < 2 * n; ++i) {
	       a[i] = drand48() - 0.5;
	       mag += a[i] * a[i];
	  }

	  /* normalize */
	  mag = 1.0 / sqrt(mag);
	  for (i = 0; i < 2 * n; ++i)
	       a0[i] = a[i] = mag * a[i];
	  
	  pln->adt->solve(pln, prblm);
	  mfft(n, a0, -1);
     
	  e1 = e2 = einf = 0.0;
	  re1 = re2 = reinf = 0.0;
	  for (i = 0; i < 2 * n; ++i) {
	       R d = a[i] - a0[i];
	       if (d < 0) d = -d;
	       e1 += d;
	       e2 += d * d;
	       if (d > einf) einf = d;

	       if (a0[i]) {
		    d /= a0[i];
		    if (d < 0) d = -d;
	       }
	       re1 += d;
	       re2 += d * d;
	       if (d > reinf) reinf = d;
	  }
	  e1 /= 2 * n;
	  e2 = sqrt(e2 / (2 * n));
	  re1 /= 2 * n;
	  re2 = sqrt(re2 / (2 * n));

	  printf("%10d %6.2e %6.2e %6.2e %6.2e %6.2e %6.2e\n",
		 n, (double)e1, (double)e2, (double)einf,
		 (double)re1, (double)re2, (double)reinf);
	  X(problem_destroy)(prblm);
     }
}
