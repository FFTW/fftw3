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
     R *a, *b;
     int i;
     R e1, n1, e2, n2, einf, ninf;

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
     b = fftw_malloc(2 * nmax * sizeof(R), OTHER);

     for (n = nmin; n <= nmax; n = 2 * n) {
	  prblm =
	       FFTW(mkproblem_dft_d)(
		    FFTW(mktensor_1d)(n, 2, 2), FFTW(mktensor)(0), a, a+1, b, b+1);
	  pln = plnr->adt->mkplan(plnr, prblm);
	  AWAKE(pln, 1);
	  for (i = 0; i < 2 * n; ++i) 
	       a[i] = drand48() - 0.5;

	  pln->adt->solve(pln, prblm);

	  /* overwrite a with its own fft */
	  mfft(n, a, -1);
     
	  e1 = e2 = einf = 0.0;
	  n1 = n2 = ninf = 0.0;
	  for (i = 0; i < 2 * n; ++i) {
	       R d;
	       d = b[i];
	       if (d < 0) d = -d;
	       n1 += d; n2 += d * d; if (d > ninf) ninf = d;

	       d = a[i] - b[i];
	       if (d < 0) d = -d;
	       e1 += d; e2 += d * d; if (d > einf) einf = d;
	  }
	  e1 /= n1;
	  e2 = sqrt(e2 / n1);
	  einf /= ninf;

	  printf("%10d %6.2e %6.2e %6.2e\n",
		 n, (double)e1, (double)e2, (double)einf);
	  X(problem_destroy)(prblm);
     }
}
