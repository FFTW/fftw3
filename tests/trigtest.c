/*
 * program to test the accuracy of trig routines. Requires libpari.
 * This program is not part of fftw.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pari/pari.h>

typedef double trigreal;
#  define COS cos
#  define SIN sin
#  define TAN tan
#  define KTRIG(x) (x)

static const trigreal K2PI = 
    KTRIG(6.2831853071795864769252867665590057683943388);

trigreal sin2pi(trigreal x);

trigreal cos2pi(trigreal x)
{
     if (x < 0.0) return cos2pi(-x);
     if (x > 0.5) return cos2pi(1.0 - x);
     if (x > 0.25) return -sin2pi(x - 0.25);
     if (x > 0.125) return sin2pi(0.25 - x);
     return COS(K2PI * x);
}

trigreal sin2pi(trigreal x)
{
     if (x < 0.0) return -sin2pi(-x);
     if (x > 0.5) return -sin2pi(1.0 - x);
     if (x > 0.25) return cos2pi(x - 0.25);
     if (x > 0.125) return cos2pi(0.25 - x);
     return SIN(K2PI * x);
}

trigreal naive_sin2pi(trigreal x)
{
     return SIN(K2PI * x);
}

trigreal naive_cos2pi(trigreal x)
{
     return COS(K2PI * x);
}

long prec = 25;

double ck(long m, long n, double (*cf)(double), GEN (*gf)(GEN, long))
{
     GEN gv, gcval, err, arg;
     double cerr, cval;
     long ltop = avma;
     
     arg = mulsr(2L * m, divrs(gpi, n));
     setlg(arg, prec);
     gv = gf(arg, prec);

     cval = cf((double)m / (double)n);
     gcval = dbltor(cval);

     err = gsub(gcval, gv);
     cerr = rtodbl(err);
     avma = ltop;
     return cerr;
}
 
int main(int argc, char *argv[])
{
     long nmin, nmax;
     long n, m;
     pari_init(500000, 2);
     mppi(prec);

     if (argc > 1)
	  nmin = atoi(argv[1]);
     else
	  nmin = 1024;

     if (argc > 2)
	  nmax = atoi(argv[2]);
     else
	  nmax = nmin;
     for (n = nmin; n <= nmax; ++n) {
	  double maxe = 0.0, nmaxe = 0.0;;
	  for (m = 0; m < n; ++m) {
	       double e;
	       e = ck(m, n, sin2pi, gsin); if (e > maxe) maxe = e;
	       e = ck(m, n, cos2pi, gcos); if (e > maxe) maxe = e;
	       e = ck(m, n, naive_sin2pi, gsin); if (e > nmaxe) nmaxe = e;
	       e = ck(m, n, naive_cos2pi, gcos); if (e > nmaxe) nmaxe = e;

	  }
	  printf("%ld %g %g\n", n, maxe, nmaxe);
     }
}
