#include "config.h"
#include "bench.h"
#include "math.h"

#define DG unsigned short
#define ACC unsigned long
#define REAL bench_real
#define BITS_IN_REAL 53 /* mantissa */

#define SHFT 16
#define RADIX 65536L
#define IRADIX (1.0 / RADIX)
#define LO(x) ((x) & (RADIX - 1))
#define HI(x) ((x) >> SHFT)
#define HI_SIGNED(x)						\
 (((-1L) >> SHFT) == (-1L) ? 					\
    (((signed long)x) >> SHFT)  /* arithmetic shift works */	\
  : ((x & (1 << (2 * SHFT - 1))) ?				\
       (((x) >> SHFT) | (RADIX * (RADIX - 1UL)))		\
     : ((x) >> SHFT)))
  
#define SEXT(x) (((x) < RADIX / 2) ? (x) : (x) + RADIX * (RADIX - 1UL))

#define ZEROEXP (-32768)

#define LEN 10

typedef struct {
     short sign;
     short expt;
     DG d[LEN]; 
} N[1];

#define EXA a->expt
#define EXB b->expt
#define EXC c->expt

#define AD a->d
#define BD b->d

#define SGNA a->sign
#define SGNB b->sign

static const N zero = {{ 1, ZEROEXP, {0} }};

static void cpy(const N a, N b)
{
     *b = *a;
}

static void fromreal(REAL x, N a)
{
     int i, e;

     cpy(zero, a);
     if (x == 0.0) return;
     
     if (x > 0) { SGNA = 1; }
     else       { SGNA = -1; x = -x; }

     e = 0;
     while (x >= 1.0) { x *= IRADIX; ++e; }
     while (x < IRADIX) { x *= RADIX; --e; }
     EXA = e;
     
     for (i = LEN - 1; i >= 0 && x != 0.0; --i) {
	  REAL y;

	  x *= RADIX;
	  y = floor(x);
	  AD[i] = (DG)y;
	  x -= y;
     }
}

static void fromshort(int x, N a)
{
     cpy(zero, a);

     if (x < 0) { x = -x; SGNA = -1; } 
     else { SGNA = 1; }
     EXA = 1;
     AD[LEN - 1] = x;
}

static void pack(DG *d, int e, int s, int l, N a)
{
     int i, j;

     for (i = l - 1; i >= 0; --i, --e) 
	  if (d[i] != 0) 
	       break;

     if (i < 0) {
	  /* number is zero */
	  cpy(zero, a);
     } else {
	  EXA = e;
	  SGNA = s;

	  if (i >= LEN - 1) {
	       for (j = LEN - 1; j >= 0; --i, --j)
		    AD[j] = d[i];
	  } else {
	       for (j = LEN - 1; i >= 0; --i, --j)
		    AD[j] = d[i];
	       for ( ; j >= 0; --j)
		    AD[j] = 0;
	  }
     }
}


/* compare absolute values */
static int abscmp(const N a, const N b)
{
     int i;
     if (EXA > EXB) return 1;
     if (EXA < EXB) return -1;
     for (i = LEN - 1; i >= 0; --i) {
	  if (AD[i] > BD[i])
	       return 1;
	  if (AD[i] < BD[i])
	       return -1;
     }
     return 0;
}

static int eq(const N a, const N b)
{
     return (SGNA == SGNB) && (abscmp(a, b) == 0);
}

/* add magnitudes, for |a| >= |b| */
static void addmag0(int s, const N a, const N b, N c)
{
     int ia, ib;
     ACC r = 0;
     DG d[LEN + 1];

     for (ia = 0, ib = EXA - EXB; ib < LEN; ++ia, ++ib) {
	  r += (ACC)AD[ia] + (ACC)BD[ib];
	  d[ia] = LO(r);
	  r = HI(r);
     }     
     for (; ia < LEN; ++ia) {
	  r += (ACC)AD[ia];
	  d[ia] = r;
	  r = HI(r);
     }
     d[ia] = r;
     pack(d, EXA + 1, s * SGNA, LEN + 1, c);
}

static void addmag(int s, const N a, const N b, N c)
{
     if (abscmp(a, b) > 0) addmag0(1, a, b, c); else addmag0(s, b, a, c);
}

/* subtract magnitudes, for |a| >= |b| */
static void submag0(int s, const N a, const N b, N c)
{
     int ia, ib;
     ACC r = 0;
     DG d[LEN];

     for (ia = 0, ib = EXA - EXB; ib < LEN; ++ia, ++ib) {
	  r += (ACC)AD[ia] - (ACC)BD[ib];
	  d[ia] = LO(r);
	  r = HI_SIGNED(r);
     }     
     for (; ia < LEN; ++ia) {
	  r += (ACC)AD[ia];
	  d[ia] = LO(r);
	  r = HI_SIGNED(r);
     }

     pack(d, EXA, s * SGNA, LEN, c);
}

static void submag(int s, const N a, const N b, N c)
{
     if (abscmp(a, b) > 0) submag0(1, a, b, c); else submag0(s, b, a, c);
}

/* c = a + b */
static void add(const N a, const N b, N c)
{
     if (SGNA == SGNB) addmag(1, a, b, c); else submag(1, a, b, c);
}

static void sub(const N a, const N b, N c)
{
     if (SGNA == SGNB) submag(-1, a, b, c); else addmag(-1, a, b, c);
}

static void mul(const N a, const N b, N c)
{
     DG d[2 * LEN];
     int i, j, k;
     ACC r;

     for (i = 0; i < LEN; ++i)
	  d[2 * i] = d[2 * i + 1] = 0;

     for (i = 0; i < LEN; ++i) {
	  ACC ai = AD[i];
	  if (ai) {
	       r = 0;
	       for (j = 0, k = i; j < LEN; ++j, ++k) {
		    r += ai * (ACC)BD[j] + (ACC)d[k];
		    d[k] = LO(r);
		    r = HI(r);
	       }
	       d[k] = LO(r);
	  }
     }

     pack(d, EXA + EXB, SGNA * SGNB, 2 * LEN, c);
}

static REAL toreal(const N a)
{
     REAL h, l;
     int i, sticky, bits, cnth, cntl;

     if (EXA != ZEROEXP) {
	  h = l = 0.0;
	  cnth = cntl = 0;

	  for (i = LEN - 1, bits = 0;
	       i >= 0 && bits < BITS_IN_REAL - SHFT + 1; --i, bits += SHFT)
	       h = h * RADIX + AD[i], ++cnth;

	  for (bits = 0; i >= 0 && bits < BITS_IN_REAL - SHFT; 
	       --i, bits += SHFT)
	       l = l * RADIX + AD[i], ++cntl;

	  for (sticky = 0; i >= 0; --i)
	       sticky |= AD[i];

	  if (sticky)
	       l += 0.5;

	  while (cntl--) l *= IRADIX;
	  h += l;
	  while (cnth--) h *= IRADIX;

	  for (i = 0; i < EXA; ++i) h *= (REAL)RADIX;
	  for (i = 0; i > EXA; --i) h *= IRADIX;
	  if (SGNA == -1) h = -h;
	  return h;
     } else {
	  return 0.0;
     }
}

static void toreals(const N a, REAL *out_, int n)
{
     N a0, b;
     int i;
     volatile REAL *out = (volatile REAL *)out_; /* force rounding */

     cpy(a, a0);
     for (i = 0; i < n; ++i, ++out) {
	  *out = toreal(a0);
	  fromreal(*out, b);
	  sub(a0, b, a0);
     }
} 

void neg(N a)
{
     SGNA = -SGNA;
}

void inv(const N a, N x)
{
     N w, z, one, two;

     fromreal(1.0 / toreal(a), x); /* initial guess */
     fromshort(1, one);
     fromshort(2, two);

     for (;;) {
	  /* Newton */
	  mul(a, x, w);
	  sub(two, w, z);
	  if (eq(one, z)) break;
	  mul(x, z, x);
     }
}


/* 2 pi */
static const N n2pi = {{
     1, 1,
     {18450, 59017, 1760, 5212, 9779, 4518, 2886, 54545, 18558, 6}
}};

/* 1 / 31! */
static const N i31fac = {{ 
     1, -7, 
     {28087, 45433, 51357, 24545, 14291, 3954, 57879, 8109, 38716, 41382}
}};


/* 1 / 32! */
static const N i32fac = {{
     1, -7,
     {52078, 60811, 3652, 39679, 37310, 47227, 28432, 57597, 13497, 1293}
}};

static void msin(const N a, N b)
{
     N a2, g, k;
     int i;

     cpy(i31fac, g);
     cpy(g, b);
     mul(a, a, a2);

     /* Taylor */
     for (i = 31; i > 1; i -= 2) {
	  fromshort(i * (i - 1), k);
	  mul(k, g, g);
	  mul(a2, b, k);
	  sub(g, k, b);
     }
     mul(a, b, b);
}

static void mcos(const N a, N b)
{
     N a2, g, k;
     int i;

     cpy(i32fac, g);
     cpy(g, b);
     mul(a, a, a2);

     /* Taylor */
     for (i = 32; i > 0; i -= 2) {
	  fromshort(i * (i - 1), k);
	  mul(k, g, g);
	  mul(a2, b, k);
	  sub(g, k, b);
     }
}

static void by2pi(REAL m, REAL n, N a)
{
     N b;

     fromreal(n, b);
     inv(b, a);
     fromreal(m, b);
     mul(a, b, a);
     mul(n2pi, a, a);
}

static void sin2pi(REAL m, REAL n, N a);
static void cos2pi(REAL m, REAL n, N a)
{
     N b;
     if (m < 0) cos2pi(-m, n, a);
     else if (m > n * 0.5) cos2pi(n - m, n, a);
     else if (m > n * 0.25) {sin2pi(m - n * 0.25, n, a); neg(a);}
     else if (m > n * 0.125) sin2pi(n * 0.25 - m, n, a);
     else { by2pi(m, n, b); mcos(b, a); }
}

static void sin2pi(REAL m, REAL n, N a)
{
     N b;
     if (m < 0)  {sin2pi(-m, n, a); neg(a);}
     else if (m > n * 0.5) {sin2pi(n - m, n, a); neg(a);}
     else if (m > n * 0.25) {cos2pi(m - n * 0.25, n, a);}
     else if (m > n * 0.125) {cos2pi(n * 0.25 - m, n, a);}
     else {by2pi(m, n, b); msin(b, a);}
}

/* FFT stuff */
static void bitrev(unsigned int n, bench_complex *a)
{
     unsigned int i, j, m;
     for (i = j = 0; i < n - 1; ++i) {
	  if (i < j) {
	       bench_real t;
	       t = c_re(a[i]); c_re(a[i]) = c_re(a[j]); c_re(a[j]) = t;
	       t = c_im(a[i]); c_im(a[i]) = c_im(a[j]); c_im(a[j]) = t;
	  }

	  /* bit reversed counter */
	  m = n; do { m >>= 1; j ^= m; } while (!(j & m));
     }
}

static void fft0(unsigned int n, N *a, int sign)
{
     unsigned int i, j, k;

     for (i = 1; i < n; i = 2 * i) {
	  for (j = 0; j < i; ++j) {
	       N wr, wi;
	       cos2pi(sign * (int)j, 2.0 * i, wr);
	       sin2pi(sign * (int)j, 2.0 * i, wi);
	       for (k = j; k < n; k += 2 * i) {
		    N *a0 = a + 2 * k;
		    N *a1 = a0 + 2 * i;
		    N r0, i0, r1, i1, t0, t1, xr, xi;
		    cpy(a0[0], r0); cpy(a0[1], i0);
		    cpy(a1[0], r1); cpy(a1[1], i1);
		    mul(r1, wr, t0); mul(i1, wi, t1); sub(t0, t1, xr);
		    mul(r1, wi, t0); mul(i1, wr, t1); add(t0, t1, xi);
		    add(r0, xr, a0[0]);  add(i0, xi, a0[1]);
		    sub(r0, xr, a1[0]);  sub(i0, xi, a1[1]);
	       }
	  }
     }
}

void mfft(unsigned int n, bench_complex *a, int sign)
{
     N *b = (N *)bench_malloc(2 * n * sizeof(N));
     int i;

     bitrev(n, a);
     for (i = 0; i < n; ++i) {
	  fromreal(c_re(a[i]), b[2 * i]);
	  fromreal(c_im(a[i]), b[2 * i + 1]);
     }
     fft0(n, b, sign);
     for (i = 0; i < n; ++i) {
	  c_re(a[i]) = toreal(b[2 * i]);
	  c_im(a[i]) = toreal(b[2 * i + 1]);
     }
     bench_free(b);
}
