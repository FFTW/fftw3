/*
 * program to test the accuracy of trig routines. Requires libpari or
 * extended precision.
 * This program is not part of fftw.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef USE_PARI
#define USE_PARI 1
#endif

typedef double trigreal;
#  define COS cos
#  define SIN sin
#  define TAN tan
#  define KTRIG(x) (x)
static const trigreal MYK2PI =
    KTRIG(6.2831853071795864769252867665590057683943388);

/**************************************************************/
/* copy-and-paste for kernel/trig.c */

#ifdef REALLY_ACCURATE
extern double fma (double X, double Y, double Z);

/* compute *x + *dx = a * b exactly */
static void dbmul(double a, double b, double *x, double *dx)
{
     *x = a * b; 
     *dx = fma(a, b, -*x);
}

/* compute *x + *dx = a / b accurately */
static void dbdiv(double a, double b, double *x, double *dx)
{
     *x = a / b;
     *dx = fma(-*x, b, a) / b;
}

static double by2pi(double m, double n)
{
     /* 2 PI rounded to IEEE-754 double precision */
     static const double rpi2 =
	  6.28318530717958623199592693708837032318115234375;
     /* 2 PI - rpi2 */
     static const double rpi2r = 
	  2.44929359829470635445213186455000211641949889184e-16;

     double x, y, z, dx, dy, dz;

     dbmul(rpi2, m, &x, &dx);      /* x + dx = rpi2 * m, exactly */
     dbmul(rpi2r, m, &y, &dy);     /* x + dx = rpi2r * m, exactly */

     /* x + dx ~ 2 PI * m, we lose roundoff in dx */
     y += dx;
     dx = y + dy;

     dbdiv(x, n, &y, &dy);      /* y + dy = x / n */
     dbdiv(dx, n, &z, &dz);     /* z + dz = dx / n */

     return ((z + dy) + dz) + y;
}
#else

static const trigreal K2PI =
    KTRIG(6.2831853071795864769252867665590057683943388);
#define by2pi(m, n) ((K2PI * (m)) / (n))

#endif

/*
 * Improve accuracy by reducing x to range [0..1/8]
 * before multiplication by 2 * PI.
 */

static trigreal sin2pi0(trigreal m, trigreal n);
static trigreal cos2pi0(trigreal m, trigreal n)
{
     if (m < 0) return cos2pi0(-m, n);
     if (m > n * 0.5) return cos2pi0(n - m, n);
     if (m > n * 0.25) return -sin2pi0(m - n * 0.25, n);
     if (m > n * 0.125) return sin2pi0(n * 0.25 - m, n);
     return COS(by2pi(m, n));
}

static trigreal sin2pi0(trigreal m, trigreal n)
{
     if (m < 0) return -sin2pi0(-m, n);
     if (m > n * 0.5) return -sin2pi0(n - m, n);
     if (m > n * 0.25) return cos2pi0(m - n * 0.25, n);
     if (m > n * 0.125) return cos2pi0(n * 0.25 - m, n);
     return SIN(by2pi(m, n));
}

trigreal cos2pi(int m, int n)
{
     return cos2pi0((trigreal)m, (trigreal)n);
}

trigreal sin2pi(int m, int n)
{
     return sin2pi0((trigreal)m, (trigreal)n);
}

trigreal tan2pi(int m, int n)
{
     trigreal dm = m, dn = n;
     /* unimplemented, unused */
     return TAN(by2pi(dm, dn));
}

/**************************************************************/
/* test code */
trigreal naive_sin2pi(int m, int n)
{
     return SIN(MYK2PI * ((trigreal) m / (trigreal) n));
}

trigreal naive_cos2pi(int m, int n)
{
     return COS(MYK2PI * ((trigreal) m / (trigreal) n));
}


#if USE_PARI
#include <pari/pari.h>

long prec = 25;

double ck(long m, long n, double (*cf) (int, int), GEN(*gf) (GEN, long))
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
#else

double ck(long m, long n, double (*cf) (int, int), 
	  long double(*gf) (long double))
{
     long double l2pi = 6.2831853071795864769252867665590057683943388L;
     return cf(m, n) - gf(l2pi * (long double)m / (long double)n);
}

#define gsin sinl
#define gcos cosl
#endif

int main(int argc, char *argv[])
{
     long nmin, nmax;
     long n, m;

#if USE_PARI
     pari_init(500000, 2);
     mppi(prec);
#endif

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
     return 0;
}
