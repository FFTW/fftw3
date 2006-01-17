/**************************************************************************/
/* NOTE to users: this is the FFTW self-test and benchmark program.
   It is probably NOT a good place to learn FFTW usage, since it has a
   lot of added complexity in order to exercise and test the full API,
   etcetera.  We suggest reading the manual. */
/**************************************************************************/

#include "bench-user.h"
#include <math.h>
#include <stdio.h>
#include <fftw3.h>
#include <string.h>

#define CONCAT(prefix, name) prefix ## name
#if defined(BENCHFFT_SINGLE)
#define FFTW(x) CONCAT(fftwf_, x)
#elif defined(BENCHFFT_LDOUBLE)
#define FFTW(x) CONCAT(fftwl_, x)
#else
#define FFTW(x) CONCAT(fftw_, x)
#endif

static const char *mkversion(void) { return FFTW(version); }
static const char *mkcc(void) { return FFTW(cc); }
static const char *mkcodelet_optim(void) { return FFTW(codelet_optim); }

BEGIN_BENCH_DOC
BENCH_DOC("name", "fftw3")
BENCH_DOCF("version", mkversion)
BENCH_DOCF("cc", mkcc)
BENCH_DOCF("codelet-optim", mkcodelet_optim)
END_BENCH_DOC 

FFTW(plan) the_plan = 0;

static const char *wisdat = "wis.dat";
unsigned the_flags = 0;
int paranoid = 0;
int usewisdom = 0;
int havewisdom = 0;
int nthreads = 1;
int amnesia = 0;

extern void install_hook(void);  /* in hook.c */
extern void uninstall_hook(void);  /* in hook.c */

void useropt(const char *arg)
{
     int x;
     double y;

     if (!strcmp(arg, "patient")) the_flags |= FFTW_PATIENT;
     else if (!strcmp(arg, "estimate")) the_flags |= FFTW_ESTIMATE;
     else if (!strcmp(arg, "estimatepat")) the_flags |= FFTW_ESTIMATE_PATIENT;
     else if (!strcmp(arg, "exhaustive")) the_flags |= FFTW_EXHAUSTIVE;
     else if (!strcmp(arg, "unaligned")) the_flags |= FFTW_UNALIGNED;
     else if (!strcmp(arg, "nosimd")) the_flags |= FFTW_NO_SIMD;
     else if (!strcmp(arg, "noindirectop")) the_flags |= FFTW_NO_INDIRECT_OP;
     else if (sscanf(arg, "flag=%d", &x) == 1) the_flags |= x;
     else if (!strcmp(arg, "paranoid")) paranoid = 1;
     else if (!strcmp(arg, "wisdom")) usewisdom = 1;
     else if (!strcmp(arg, "amnesia")) amnesia = 1;
     else if (sscanf(arg, "nthreads=%d", &x) == 1) nthreads = x;
     else if (sscanf(arg, "timelimit=%lg", &y) == 1) {
	  FFTW(set_timelimit)(y);
     }

     else fprintf(stderr, "unknown user option: %s.  Ignoring.\n", arg);
}

void rdwisdom(void)
{
     FILE *f;
     double tim;
     int success = 0;

     if (havewisdom) return;

#ifdef HAVE_THREADS
     BENCH_ASSERT(FFTW(init_threads)());
     FFTW(plan_with_nthreads)(nthreads);
#endif

     if (!usewisdom) return;

     timer_start(USER_TIMER);
     if ((f = fopen(wisdat, "r"))) {
	  if (!FFTW(import_wisdom_from_file)(f))
	       fprintf(stderr, "bench: ERROR reading wisdom\n");
	  else
	       success = 1;
	  fclose(f);
     }
     tim = timer_stop(USER_TIMER);

     if (success) {
	  if (verbose > 1) printf("READ WISDOM (%g seconds): ", tim);
	  
	  if (verbose > 3)
	       FFTW(export_wisdom_to_file)(stdout);
	  if (verbose > 1)
	       printf("\n");
     }
     havewisdom = 1;
}

void wrwisdom(void)
{
     FILE *f;
     double tim;
     if (!havewisdom) return;

     timer_start(USER_TIMER);
     if ((f = fopen(wisdat, "w"))) {
	  FFTW(export_wisdom_to_file)(f);
	  fclose(f);
     }
     tim = timer_stop(USER_TIMER);
     if (verbose > 1) printf("write wisdom took %g seconds\n", tim);
}

static FFTW(iodim) *bench_tensor_to_fftw_iodim(bench_tensor *t)
{
     FFTW(iodim) *d;
     int i;

     BENCH_ASSERT(t->rnk >= 0);
     if (t->rnk == 0) return 0;
     
     d = (FFTW(iodim) *)bench_malloc(sizeof(FFTW(iodim)) * t->rnk);
     for (i = 0; i < t->rnk; ++i) {
	  d[i].n = t->dims[i].n;
	  d[i].is = t->dims[i].is;
	  d[i].os = t->dims[i].os;
     }

     return d;
}

static void extract_reim_split(int sign, int size, bench_real *p,
			       bench_real **r, bench_real **i)
{
     if (sign == FFTW_FORWARD) {
          *r = p + 0;
          *i = p + size;
     } else {
          *r = p + size;
          *i = p + 0;
     }
}

static int sizeof_problem(bench_problem *p)
{
     return tensor_sz(p->sz) * tensor_sz(p->vecsz);
}

/* ouch */
static int expressible_as_api_many(bench_tensor *t)
{
     int i;

     BENCH_ASSERT(FINITE_RNK(t->rnk));

     i = t->rnk - 1;
     while (--i >= 0) {
	  bench_iodim *d = t->dims + i;
	  if (d[0].is % d[1].is) return 0;
	  if (d[0].os % d[1].os) return 0;
     }
     return 1;
}

static int *mkn(bench_tensor *t)
{
     int *n = (int *) bench_malloc(sizeof(int *) * t->rnk);
     int i;
     for (i = 0; i < t->rnk; ++i) 
	  n[i] = t->dims[i].n;
     return n;
}

static void mknembed_many(bench_tensor *t, int **inembedp, int **onembedp)
{
     int i;
     bench_iodim *d;
     int *inembed = (int *) bench_malloc(sizeof(int *) * t->rnk);
     int *onembed = (int *) bench_malloc(sizeof(int *) * t->rnk);

     BENCH_ASSERT(FINITE_RNK(t->rnk));
     *inembedp = inembed; *onembedp = onembed;

     i = t->rnk - 1;
     while (--i >= 0) {
	  d = t->dims + i;
	  inembed[i+1] = d[0].is / d[1].is;
	  onembed[i+1] = d[0].os / d[1].os;
     }
}

/* try to use the most appropriate API function.  Big mess. */

static int imax(int a, int b) { return (a > b ? a : b); }

static int halfish_sizeof_problem(bench_problem *p)
{
     int n2 = sizeof_problem(p);
     if (FINITE_RNK(p->sz->rnk) && p->sz->rnk > 0)
          n2 = (n2 / imax(p->sz->dims[p->sz->rnk - 1].n, 1)) *
               (p->sz->dims[p->sz->rnk - 1].n / 2 + 1);
     return n2;
}

static FFTW(plan) mkplan_real_split(bench_problem *p, int flags)
{
     FFTW(plan) pln;
     bench_tensor *sz = p->sz, *vecsz = p->vecsz;
     FFTW(iodim) *dims, *howmany_dims;
     bench_real *ri, *ii, *ro, *io;
     int n2 = halfish_sizeof_problem(p);

     extract_reim_split(FFTW_FORWARD, n2, (bench_real *) p->in, &ri, &ii);
     extract_reim_split(FFTW_FORWARD, n2, (bench_real *) p->out, &ro, &io);

     dims = bench_tensor_to_fftw_iodim(sz);
     howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
     if (p->sign < 0) {
	  if (verbose > 2) printf("using plan_guru_split_dft_r2c\n");
	  pln = FFTW(plan_guru_split_dft_r2c)(sz->rnk, dims,
					vecsz->rnk, howmany_dims,
					ri, ro, io, flags);
     }
     else {
	  if (verbose > 2) printf("using plan_guru_split_dft_c2r\n");
	  pln = FFTW(plan_guru_split_dft_c2r)(sz->rnk, dims,
					vecsz->rnk, howmany_dims,
					ri, ii, ro, flags);
     }
     bench_free(dims);
     bench_free(howmany_dims);
     return pln;
}

static FFTW(plan) mkplan_real_interleaved(bench_problem *p, int flags)
{
     FFTW(plan) pln;
     bench_tensor *sz = p->sz, *vecsz = p->vecsz;

     if (vecsz->rnk == 0 && tensor_unitstridep(sz) 
	 && tensor_real_rowmajorp(sz, p->sign, p->in_place)) 
	  goto api_simple;
     
     if (vecsz->rnk == 1 && expressible_as_api_many(sz))
	  goto api_many;

     goto api_guru;

 api_simple:
     switch (sz->rnk) {
	 case 1:
	      if (p->sign < 0) {
		   if (verbose > 2) printf("using plan_dft_r2c_1d\n");
		   return FFTW(plan_dft_r2c_1d)(sz->dims[0].n, 
						(bench_real *) p->in, 
						(bench_complex *) p->out,
						flags);
	      }
	      else {
		   if (verbose > 2) printf("using plan_dft_c2r_1d\n");
		   return FFTW(plan_dft_c2r_1d)(sz->dims[0].n, 
						(bench_complex *) p->in, 
						(bench_real *) p->out,
						flags);
	      }
	      break;
	 case 2:
	      if (p->sign < 0) {
		   if (verbose > 2) printf("using plan_dft_r2c_2d\n");
		   return FFTW(plan_dft_r2c_2d)(sz->dims[0].n, sz->dims[1].n,
						(bench_real *) p->in, 
						(bench_complex *) p->out,
						flags);
	      }
	      else {
		   if (verbose > 2) printf("using plan_dft_c2r_2d\n");
		   return FFTW(plan_dft_c2r_2d)(sz->dims[0].n, sz->dims[1].n,
						(bench_complex *) p->in, 
						(bench_real *) p->out,
						flags);
	      }
	      break;
	 case 3:
	      if (p->sign < 0) {
		   if (verbose > 2) printf("using plan_dft_r2c_3d\n");
		   return FFTW(plan_dft_r2c_3d)(
			sz->dims[0].n, sz->dims[1].n, sz->dims[2].n,
			(bench_real *) p->in, (bench_complex *) p->out,
			flags);
	      }
	      else {
		   if (verbose > 2) printf("using plan_dft_c2r_3d\n");
		   return FFTW(plan_dft_c2r_3d)(
			sz->dims[0].n, sz->dims[1].n, sz->dims[2].n,
			(bench_complex *) p->in, (bench_real *) p->out,
			flags);
	      }
	      break;
	 default: {
	      int *n = mkn(sz);
	      if (p->sign < 0) {
		   if (verbose > 2) printf("using plan_dft_r2c\n");
		   pln = FFTW(plan_dft_r2c)(sz->rnk, n,
					    (bench_real *) p->in, 
					    (bench_complex *) p->out,
					    flags);
	      }
	      else {
		   if (verbose > 2) printf("using plan_dft_c2r\n");
		   pln = FFTW(plan_dft_c2r)(sz->rnk, n,
					    (bench_complex *) p->in, 
					    (bench_real *) p->out,
					    flags);
	      }
	      bench_free(n);
	      return pln;
	 }
     }

 api_many:
     {
	  int *n, *inembed, *onembed;
	  BENCH_ASSERT(vecsz->rnk == 1);
	  n = mkn(sz);
	  mknembed_many(sz, &inembed, &onembed);
	  if (p->sign < 0) {
	       if (verbose > 2) printf("using plan_many_dft_r2c\n");
	       pln = FFTW(plan_many_dft_r2c)(
		    sz->rnk, n, vecsz->dims[0].n, 
		    (bench_real *) p->in, inembed,
		    sz->dims[sz->rnk - 1].is, vecsz->dims[0].is,
		    (bench_complex *) p->out, onembed,
		    sz->dims[sz->rnk - 1].os, vecsz->dims[0].os,
		    flags);
	  }
	  else {
	       if (verbose > 2) printf("using plan_many_dft_c2r\n");
	       pln = FFTW(plan_many_dft_c2r)(
		    sz->rnk, n, vecsz->dims[0].n, 
		    (bench_complex *) p->in, inembed,
		    sz->dims[sz->rnk - 1].is, vecsz->dims[0].is,
		    (bench_real *) p->out, onembed,
		    sz->dims[sz->rnk - 1].os, vecsz->dims[0].os,
		    flags);
	  }
	  bench_free(n); bench_free(inembed); bench_free(onembed);
	  return pln;
     }

 api_guru:
     {
	  FFTW(iodim) *dims, *howmany_dims;

	  if (p->sign < 0) {
	       dims = bench_tensor_to_fftw_iodim(sz);
	       howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
	       if (verbose > 2) printf("using plan_guru_dft_r2c\n");
	       pln = FFTW(plan_guru_dft_r2c)(sz->rnk, dims,
					     vecsz->rnk, howmany_dims,
					     (bench_real *) p->in,
					     (bench_complex *) p->out,
					     flags);
	  }
	  else {
	       dims = bench_tensor_to_fftw_iodim(sz);
	       howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
	       if (verbose > 2) printf("using plan_guru_dft_c2r\n");
	       pln = FFTW(plan_guru_dft_c2r)(sz->rnk, dims,
					     vecsz->rnk, howmany_dims,
					     (bench_complex *) p->in,
					     (bench_real *) p->out,
					     flags);
	  }
	  bench_free(dims);
	  bench_free(howmany_dims);
	  return pln;
     }
}

static FFTW(plan) mkplan_real(bench_problem *p, int flags)
{
     if (p->split)
	  return mkplan_real_split(p, flags);
     else
	  return mkplan_real_interleaved(p, flags);
}

static FFTW(plan) mkplan_complex_split(bench_problem *p, int flags)
{
     FFTW(plan) pln;
     bench_tensor *sz = p->sz, *vecsz = p->vecsz;
     FFTW(iodim) *dims, *howmany_dims;
     bench_real *ri, *ii, *ro, *io;
     int n = sizeof_problem(p);

     extract_reim_split(p->sign, n, (bench_real *) p->in, &ri, &ii);
     extract_reim_split(p->sign, n, (bench_real *) p->out, &ro, &io);

     dims = bench_tensor_to_fftw_iodim(sz);
     howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
     if (verbose > 2) printf("using plan_guru_split_dft\n");
     pln = FFTW(plan_guru_split_dft)(sz->rnk, dims,
			       vecsz->rnk, howmany_dims,
			       ri, ii, ro, io, flags);
     bench_free(dims);
     bench_free(howmany_dims);
     return pln;
}

static FFTW(plan) mkplan_complex_interleaved(bench_problem *p, int flags)
{
     FFTW(plan) pln;
     bench_tensor *sz = p->sz, *vecsz = p->vecsz;

     if (vecsz->rnk == 0 && tensor_unitstridep(sz) && tensor_rowmajorp(sz)) 
	  goto api_simple;
     
     if (vecsz->rnk == 1 && expressible_as_api_many(sz))
	  goto api_many;

     goto api_guru;

 api_simple:
     switch (sz->rnk) {
	 case 1:
	      if (verbose > 2) printf("using plan_dft_1d\n");
	      return FFTW(plan_dft_1d)(sz->dims[0].n, 
				       (bench_complex *) p->in,
				       (bench_complex *) p->out, 
				       p->sign, flags);
	      break;
	 case 2:
	      if (verbose > 2) printf("using plan_dft_2d\n");
	      return FFTW(plan_dft_2d)(sz->dims[0].n, sz->dims[1].n,
				       (bench_complex *) p->in,
				       (bench_complex *) p->out, 
				       p->sign, flags);
	      break;
	 case 3:
	      if (verbose > 2) printf("using plan_dft_3d\n");
	      return FFTW(plan_dft_3d)(
		   sz->dims[0].n, sz->dims[1].n, sz->dims[2].n,
		   (bench_complex *) p->in, (bench_complex *) p->out, 
		   p->sign, flags);
	      break;
	 default: {
	      int *n = mkn(sz);
	      if (verbose > 2) printf("using plan_dft\n");
	      pln = FFTW(plan_dft)(sz->rnk, n, 
				   (bench_complex *) p->in, 
				   (bench_complex *) p->out, p->sign, flags);
	      bench_free(n);
	      return pln;
	 }
     }

 api_many:
     {
	  int *n, *inembed, *onembed;
	  BENCH_ASSERT(vecsz->rnk == 1);
	  n = mkn(sz);
	  mknembed_many(sz, &inembed, &onembed);
	  if (verbose > 2) printf("using plan_many_dft\n");
	  pln = FFTW(plan_many_dft)(
	       sz->rnk, n, vecsz->dims[0].n, 
	       (bench_complex *) p->in, 
	       inembed, sz->dims[sz->rnk - 1].is, vecsz->dims[0].is,
	       (bench_complex *) p->out,
	       onembed, sz->dims[sz->rnk - 1].os, vecsz->dims[0].os,
	       p->sign, flags);
	  bench_free(n); bench_free(inembed); bench_free(onembed);
	  return pln;
     }

 api_guru:
     {
	  FFTW(iodim) *dims, *howmany_dims;

	  dims = bench_tensor_to_fftw_iodim(sz);
	  howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
	  if (verbose > 2) printf("using plan_guru_dft\n");
	  pln = FFTW(plan_guru_dft)(sz->rnk, dims,
				    vecsz->rnk, howmany_dims,
				    (bench_complex *) p->in,
				    (bench_complex *) p->out,
				    p->sign, flags);
	  bench_free(dims);
	  bench_free(howmany_dims);
	  return pln;
     }
}

static FFTW(plan) mkplan_complex(bench_problem *p, int flags)
{
     if (p->split)
	  return mkplan_complex_split(p, flags);
     else
	  return mkplan_complex_interleaved(p, flags);
}

static FFTW(plan) mkplan_r2r(bench_problem *p, int flags)
{
     FFTW(plan) pln;
     bench_tensor *sz = p->sz, *vecsz = p->vecsz;
     FFTW(r2r_kind) *k;

     k = (FFTW(r2r_kind) *) bench_malloc(sizeof(FFTW(r2r_kind)) * sz->rnk);
     {
	  int i;
	  for (i = 0; i < sz->rnk; ++i)
	       switch (p->k[i]) {
		   case R2R_R2HC: k[i] = FFTW_R2HC; break;
		   case R2R_HC2R: k[i] = FFTW_HC2R; break;
		   case R2R_DHT: k[i] = FFTW_DHT; break;
		   case R2R_REDFT00: k[i] = FFTW_REDFT00; break;
		   case R2R_REDFT01: k[i] = FFTW_REDFT01; break;
		   case R2R_REDFT10: k[i] = FFTW_REDFT10; break;
		   case R2R_REDFT11: k[i] = FFTW_REDFT11; break;
		   case R2R_RODFT00: k[i] = FFTW_RODFT00; break;
		   case R2R_RODFT01: k[i] = FFTW_RODFT01; break;
		   case R2R_RODFT10: k[i] = FFTW_RODFT10; break;
		   case R2R_RODFT11: k[i] = FFTW_RODFT11; break;
		   default: BENCH_ASSERT(0);
	       }
     }

     if (vecsz->rnk == 0 && tensor_unitstridep(sz) && tensor_rowmajorp(sz)) 
	  goto api_simple;
     
     if (vecsz->rnk == 1 && expressible_as_api_many(sz))
	  goto api_many;

     goto api_guru;

 api_simple:
     switch (sz->rnk) {
	 case 1:
	      if (verbose > 2) printf("using plan_r2r_1d\n");
	      pln = FFTW(plan_r2r_1d)(sz->dims[0].n, 
				      (bench_real *) p->in,
				      (bench_real *) p->out, 
				      k[0], flags);
	      goto done;
	 case 2:
	      if (verbose > 2) printf("using plan_r2r_2d\n");
	      pln = FFTW(plan_r2r_2d)(sz->dims[0].n, sz->dims[1].n,
				      (bench_real *) p->in,
				      (bench_real *) p->out, 
				      k[0], k[1], flags);
	      goto done;
	 case 3:
	      if (verbose > 2) printf("using plan_r2r_3d\n");
	      pln = FFTW(plan_r2r_3d)(
		   sz->dims[0].n, sz->dims[1].n, sz->dims[2].n,
		   (bench_real *) p->in, (bench_real *) p->out, 
		   k[0], k[1], k[2], flags);
	      goto done;
	 default: {
	      int *n = mkn(sz);
	      if (verbose > 2) printf("using plan_r2r\n");
	      pln = FFTW(plan_r2r)(sz->rnk, n,
				   (bench_real *) p->in, (bench_real *) p->out,
				   k, flags);
	      bench_free(n);
	      goto done;
	 }
     }

 api_many:
     {
	  int *n, *inembed, *onembed;
	  BENCH_ASSERT(vecsz->rnk == 1);
	  n = mkn(sz);
	  mknembed_many(sz, &inembed, &onembed);
	  if (verbose > 2) printf("using plan_many_r2r\n");
	  pln = FFTW(plan_many_r2r)(
	       sz->rnk, n, vecsz->dims[0].n, 
	       (bench_real *) p->in,
	       inembed, sz->dims[sz->rnk - 1].is, vecsz->dims[0].is,
	       (bench_real *) p->out,
	       onembed, sz->dims[sz->rnk - 1].os, vecsz->dims[0].os,
	       k, flags);
	  bench_free(n); bench_free(inembed); bench_free(onembed);
	  goto done;
     }

 api_guru:
     {
	  FFTW(iodim) *dims, *howmany_dims;

	  dims = bench_tensor_to_fftw_iodim(sz);
	  howmany_dims = bench_tensor_to_fftw_iodim(vecsz);
	  if (verbose > 2) printf("using plan_guru_r2r\n");
	  pln = FFTW(plan_guru_r2r)(sz->rnk, dims,
				    vecsz->rnk, howmany_dims,
				    (bench_real *) p->in, 
				    (bench_real *) p->out, k, flags);
	  bench_free(dims);
	  bench_free(howmany_dims);
	  goto done;
     }
     
 done:
     bench_free(k);
     return pln;
}

static FFTW(plan) mkplan(bench_problem *p, int flags)
{
     switch (p->kind) {
	 case PROBLEM_COMPLEX:	  return mkplan_complex(p, flags);
	 case PROBLEM_REAL:	  return mkplan_real(p, flags);
	 case PROBLEM_R2R:        return mkplan_r2r(p, flags);
	 default: BENCH_ASSERT(0); return 0;
     }
}

static unsigned preserve_input_flags(bench_problem *p)
{
     /*
      * fftw3 cannot preserve input for multidimensional c2r transforms.
      * Enforce FFTW_DESTROY_INPUT
      */
     if (p->kind == PROBLEM_REAL && 
	 p->sign > 0 && 
	 !p->in_place && 
	 p->sz->rnk > 1)
	  p->destroy_input = 1;

     if (p->destroy_input)
	  return FFTW_DESTROY_INPUT;
     else
	  return FFTW_PRESERVE_INPUT;
}

int can_do(bench_problem *p)
{
     double tim;

     if (verbose > 2 && p->pstring)
	  printf("Planning %s...\n", p->pstring);
     rdwisdom();

     timer_start(USER_TIMER);
     the_plan = mkplan(p, preserve_input_flags(p) | the_flags | FFTW_ESTIMATE);
     tim = timer_stop(USER_TIMER);
     if (verbose > 2) printf("estimate-planner time: %g s\n", tim);

     if (the_plan) {
	  FFTW(destroy_plan)(the_plan);
	  return 1;
     }
     return 0;
}

void setup(bench_problem *p)
{
     double tim;

     if (amnesia)
	  FFTW(forget_wisdom)();

     /* Regression test: check that fftw_malloc exists and links
      * properly */
     FFTW(free(FFTW(malloc(42))));

     rdwisdom();
     install_hook();

#ifdef HAVE_THREADS
     if (verbose > 1 && nthreads > 1) printf("NTHREADS = %d\n", nthreads);
#endif

     timer_start(USER_TIMER);
     the_plan = mkplan(p, preserve_input_flags(p) | the_flags);
     tim = timer_stop(USER_TIMER);
     if (verbose > 1) printf("planner time: %g s\n", tim);

     BENCH_ASSERT(the_plan);

     if (verbose > 1) {
	  double add, mul, fma;
	  FFTW(print_plan)(the_plan);
	  printf("\n");
	  FFTW(flops)(the_plan, &add, &mul, &fma);
	  printf("flops: %0.0f add, %0.0f mul, %0.0f fma\n", add, mul, fma);
	  printf("estimated cost: %f\n", FFTW(estimate_cost)(the_plan));
     }
}


void doit(int iter, bench_problem *p)
{
     int i;
     FFTW(plan) q = the_plan;

     UNUSED(p);
     for (i = 0; i < iter; ++i) 
	  FFTW(execute)(q);
}

void done(bench_problem *p)
{
     UNUSED(p);

     FFTW(destroy_plan)(the_plan);
     uninstall_hook();
}

void cleanup(void)
{
     wrwisdom();
#ifdef HAVE_THREADS
     FFTW(cleanup_threads)();
#else
     FFTW(cleanup)();
#endif

#    ifdef FFTW_DEBUG_MALLOC
     {
	  /* undocumented memory checker */
	  FFTW_EXTERN void FFTW(malloc_print_minfo)(int v);
	  FFTW(malloc_print_minfo)(verbose);
     }
#    endif
}
