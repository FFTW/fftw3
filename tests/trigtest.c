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


trigreal naive_sin2pi(int m, uint n)
{
     return SIN(K2PI * ((trigreal) m / (trigreal) n));
}

trigreal naive_cos2pi(int m, uint n)
{
     return COS(K2PI * ((trigreal) m / (trigreal) n));
}

static const trigreal KPIO2 =
    KTRIG(1.57079632679489661923132169163975144209858469968);

static trigreal sin2pi1(int m, uint n, int k);
static trigreal cos2pi1(int m, uint n, int k)
{
     if (m < 0) return cos2pi1(-m, n, -k);
     if (2 * m > n) return cos2pi1(n - m, n, -k);
     if (4 * m > n) return -sin2pi1(m - n / 4, n, k + 4 * (n / 4) - n);
     if (8 * m > n) return sin2pi1(n / 4 - m, n, -k + n - 4 * (n / 4));
     return COS(KPIO2 * (((trigreal) k + 4.0 * (trigreal)m) / (trigreal)n));
}

static trigreal sin2pi1(int m, uint n, int k)
{
     if (m < 0) return -sin2pi1(-m, n, -k);
     if (2 * m > n) return -sin2pi1(n - m, n, -k);
     if (4 * m > n) return cos2pi1(m - n / 4, n, k + 4 * (n / 4) - n);
     if (8 * m > n) return cos2pi1(n / 4 - m, n, -k + n - 4 * (n / 4));
     return SIN(KPIO2 * (((trigreal) k + 4.0 * (trigreal)m) / (trigreal)n));
}

trigreal cos2pi(int m, uint n)
{
     return cos2pi1(m, n, 0);
}

trigreal sin2pi(int m, uint n)
{
     return sin2pi1(m, n, 0);
}

long prec = 25;

double ck(long m, long n, double (*cf) (int, uint), GEN(*gf) (GEN, long))
{
     GEN gv, gcval, err, arg;
     double cerr, cval;
     long ltop = avma;

     arg = mulsr(2L * m, divrs(gpi, n));
     setlg(arg, prec);
     gv = gf(arg, prec);

     cval = cf(m, n);
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
	       e = ck(m, n, sin2pi, gsin);
	       if (e > maxe)
		    maxe = e;
	       e = ck(m, n, cos2pi, gcos);
	       if (e > maxe)
		    maxe = e;
	       e = ck(m, n, naive_sin2pi, gsin);
	       if (e > nmaxe)
		    nmaxe = e;
	       e = ck(m, n, naive_cos2pi, gcos);
	       if (e > nmaxe)
		    nmaxe = e;

	  }
	  printf("%ld %g %g\n", n, maxe, nmaxe);
     }
}
