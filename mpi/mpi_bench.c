/**************************************************************************/
/* NOTE to users: this is the FFTW-MPI self-test and benchmark program.
   It is probably NOT a good place to learn FFTW usage, since it has a
   lot of added complexity in order to exercise and test the full API,
   etcetera.  We suggest reading the manual. */
/**************************************************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "fftw3_mpi.h"
#include "fftw_bench.h"

#if defined(BENCHFFT_SINGLE)
#  define BENCH_MPI_TYPE MPI_FLOAT
#elif defined(BENCHFFT_LDOUBLE)
#  define BENCH_MPI_TYPE MPI_LONG_DOUBLE
#else
#  define BENCH_MPI_TYPE MPI_DOUBLE
#endif

static const char *mkversion(void) { return FFTW(version); }
static const char *mkcc(void) { return FFTW(cc); }
static const char *mkcodelet_optim(void) { return FFTW(codelet_optim); }
static const char *mknproc(void) {
     static char buf[32];
     int ncpus;
     MPI_Comm_size(MPI_COMM_WORLD, &ncpus);
#ifdef HAVE_SNPRINTF
     snprintf(buf, 32, "%d", ncpus);
#else
     sprintf(buf, "%d", ncpus);
#endif
     return buf;
}

BEGIN_BENCH_DOC
BENCH_DOC("name", "fftw3_mpi")
BENCH_DOCF("version", mkversion)
BENCH_DOCF("cc", mkcc)
BENCH_DOCF("codelet-optim", mkcodelet_optim)
BENCH_DOCF("nproc", mknproc)
END_BENCH_DOC 

static ptrdiff_t ni, local_ni, local_starti, nresti; /* local in size */
static ptrdiff_t no, local_no, local_starto, nresto; /* local out size */
bench_real *local_in = 0, *local_out = 0;
FFTW(plan) plan_scramble_in = 0, plan_unscramble_out = 0;

int n_pes = 1, my_pe = 0;
int *recv_cnt = 0, *recv_off; /* for MPI_Gatherv */

static void alloc_local(ptrdiff_t nreal, int inplace)
{
     bench_free(local_in);
     if (local_out != local_in) bench_free(local_out);
     local_in = local_out = 0;
     if (nreal > 0) {
	  ptrdiff_t i;
	  local_in = (bench_real*) bench_malloc(nreal * sizeof(bench_real));
	  if (inplace)
	       local_out = local_in;
	  else
	       local_out = (bench_real*) bench_malloc(nreal * sizeof(bench_real));
	  for (i = 0; i < nreal; ++i) local_in[i] = 0.0;
     }
}

void after_problem_rcopy_from(bench_problem *p, bench_real *ri)
{
     ptrdiff_t i, j;
     UNUSED(p);
     for (i = 0; i < local_ni; ++i)
	  for (j = 0; j < nresti; ++j)
	       local_in[i*nresti + j] = ri[(i+local_starti)*nresti + j];
     if (plan_scramble_in) FFTW(execute)(plan_scramble_in);
}

static void collect_data(ptrdiff_t n, ptrdiff_t vn,
			 bench_real *out)
{
     ptrdiff_t block;
     int i, tot = 0;
     block = (n + n_pes - 1) / n_pes; /* FFTW_MPI_DEFAULT_BLOCK */
     for (i = 0; i < n / block; ++i)
	  recv_cnt[i] = block * vn;
     if (i < n_pes)
	  recv_cnt[i] = (n - block * i) * vn;
     for (i++; i < n_pes; ++i)
	  recv_cnt[i] = 0;
     recv_off[0] = 0;
     for (i = 1; i < n_pes; ++i)
	  recv_off[i] = recv_off[i-1] + recv_cnt[i-1];
     for (i = 0; i < n_pes; ++i) tot += recv_cnt[i];
     MPI_Gatherv(local_out, recv_cnt[my_pe], BENCH_MPI_TYPE,
		 out, recv_cnt, recv_off, BENCH_MPI_TYPE,
		 0, MPI_COMM_WORLD);
     MPI_Bcast(out, tot, BENCH_MPI_TYPE, 0, MPI_COMM_WORLD);
}

void after_problem_rcopy_to(bench_problem *p, bench_real *ro)
{
     UNUSED(p);
     if (plan_unscramble_out) FFTW(execute)(plan_unscramble_out);
     collect_data(no, nresto, ro);
}

void after_problem_ccopy_from(bench_problem *p, bench_real *ri, bench_real *ii)
{
     after_problem_rcopy_from(p, ri);
}

void after_problem_ccopy_to(bench_problem *p, bench_real *ro, bench_real *io)
{
     after_problem_rcopy_to(p, ro);
}

static FFTW(plan) mkplan_transpose_local(ptrdiff_t nx, ptrdiff_t ny, 
					 ptrdiff_t vn, 
					 bench_real *in, bench_real *out)
{
     FFTW(iodim64) hdims[3];
     FFTW(r2r_kind) k[3];
     FFTW(plan) pln;

     hdims[0].n = nx;
     hdims[0].is = ny * vn;
     hdims[0].os = vn;
     hdims[1].n = ny;
     hdims[1].is = vn;
     hdims[1].os = nx * vn;
     hdims[2].n = vn;
     hdims[2].is = 1;
     hdims[2].os = 1;
     k[0] = k[1] = k[2] = FFTW_R2HC;
     pln = FFTW(plan_guru64_r2r)(0, 0, 3, hdims, in, out, k, FFTW_ESTIMATE);
     BENCH_ASSERT(pln != 0);
     return pln;
}

/* make plan_scramble_in and plan_unscramble_out, if necessary;
   assumes local_ni etc. have been initialized. */
static void make_scramblers(int rnk, ptrdiff_t nx, ptrdiff_t ny,
			    ptrdiff_t vn, int flags)
{
     FFTW(destroy_plan)(plan_scramble_in); plan_scramble_in = 0;
     FFTW(destroy_plan)(plan_unscramble_out); plan_unscramble_out = 0;
     if (rnk > 1) {
	  if (flags & FFTW_MPI_SCRAMBLED_IN)
	       plan_scramble_in = mkplan_transpose_local(local_ni, ny, vn,
							 local_in, local_in);
	  if (flags & FFTW_MPI_SCRAMBLED_OUT) {
	       if (flags & FFTW_MPI_TRANSPOSED)
		    plan_unscramble_out = mkplan_transpose_local
			 (nx, local_no, vn, local_out, local_out);
	       else
		    plan_unscramble_out = mkplan_transpose_local
			 (ny, local_ni, vn, local_out, local_out);
	  }
     }
     /* else: don't know how to unscramble rnk == 1 */
}

static int tensor_rowmajor_transposedp(bench_tensor *t)
{
     bench_iodim *d;
     int i;

     BENCH_ASSERT(FINITE_RNK(t->rnk));
     if (t->rnk < 2)
	  return 0;

     d = t->dims;
     if (d[0].is != d[1].is * d[1].n
	 || d[0].os != d[1].is
	 || d[1].os != d[0].os * d[0].n)
	  return 0;
     if (t->rnk > 2 && d[1].is != d[2].is * d[2].n)
	  return 0;
     for (i = 2; i + 1 < t->rnk; ++i) {
          d = t->dims + i;
          if (d[0].is != d[1].is * d[1].n
	      || d[0].os != d[1].os * d[1].n)
               return 0;
     }

     if (t->rnk > 2 && t->dims[t->rnk-1].is != t->dims[t->rnk-1].os)
	  return 0;
     return 1;
}

static int tensor_contiguousp(bench_tensor *t, int s)
{
     return (t->dims[t->rnk-1].is == s
	     && ((tensor_rowmajorp(t) && 
		  t->dims[t->rnk-1].is == t->dims[t->rnk-1].os)
		 || tensor_rowmajor_transposedp(t)));
}

static FFTW(plan) mkplan_complex(bench_problem *p, int flags)
{
     FFTW(plan) pln = 0;
     int i; 
     ptrdiff_t *n;
     ptrdiff_t vn, ntot;

     vn = p->vecsz->rnk == 1 ? p->vecsz->dims[0].n : 1;

     if (p->sz->rnk < 1
	 || p->split
	 || !tensor_contiguousp(p->sz, vn)
	 || p->vecsz->rnk > 1
	 || (p->vecsz->rnk == 1 && (p->vecsz->dims[0].is != 1
				    || p->vecsz->dims[0].os != 1)))
	  return 0;

     if (tensor_rowmajor_transposedp(p->sz))
	  flags |= FFTW_MPI_TRANSPOSED;

     n = (ptrdiff_t *) bench_malloc(sizeof(ptrdiff_t) * p->sz->rnk);
     for (i = 0; i < p->sz->rnk; ++i)
	  n[i] = p->sz->dims[i].n;
     
     /* TODO: exercise full API? */

     ntot = FFTW(mpi_local_size_many_transposed)
	  (p->sz->rnk, n, vn,
	   FFTW_MPI_DEFAULT_BLOCK, FFTW_MPI_DEFAULT_BLOCK,
	   MPI_COMM_WORLD,
	   &local_ni, &local_starti, &local_no, &local_starto);
     alloc_local(ntot * 2, p->in == p->out);
     ntot = vn * 2;
     for (i = 2; i < p->sz->rnk; ++i) ntot *= n[i];
     if (p->sz->rnk > 1) {
	  ni = n[0];
	  nresti = n[1] * ntot;
	  if (flags & FFTW_MPI_TRANSPOSED) {
	       no = n[1];
	       nresto = n[0] * ntot;
	  }
	  else { /* output is same shape as input */
	       no = ni;
	       local_no = local_ni;
	       local_starto = local_starti;
	       nresto = nresti;
	  }
     }
     else /* rnk == 1 */ {
	  ni = no = n[0];
	  nresti = nresto = ntot;
	  local_no = local_ni;
	  local_starto = local_starti;
     }

     pln = FFTW(mpi_plan_many_dft)(p->sz->rnk, n, vn, 
				   FFTW_MPI_DEFAULT_BLOCK,
				   FFTW_MPI_DEFAULT_BLOCK,
				   (FFTW(complex) *) local_in, 
				   (FFTW(complex) *) local_out,
				   MPI_COMM_WORLD, p->sign, flags);

     if (pln) make_scramblers(p->sz->rnk, n[0], n[1], ntot, flags);

     bench_free(n);

     return pln;
}

static FFTW(plan) mkplan_real(bench_problem *p, int flags)
{
     return 0;
}

static FFTW(plan) mkplan_transpose(bench_problem *p, int flags)
{
     ptrdiff_t ntot, nx, ny, vn;
     int ix, iy, i;
     const bench_iodim *d = p->vecsz->dims;
     FFTW(plan) pln;

     if (p->vecsz->rnk == 3) {
	  for (i = 0; i < 3; ++i)
	       if (d[i].is == 1 && d[i].is == 1) {
		    vn = d[i].n;
		    ix = (i + 1) % 3;
		    iy = (i + 2) % 3;
		    break;
	       }
	  if (i == 3) return 0;
     }
     else {
	  vn = 1;
	  ix = 0;
	  iy = 1;
     }

     if (d[ix].is == d[iy].n * vn && d[ix].os == vn
	 && d[iy].os == d[ix].n * vn && d[iy].is == vn) {
	  nx = d[ix].n;
	  ny = d[iy].n;
     }
     else if (d[iy].is == d[ix].n * vn && d[iy].os == vn
	      && d[ix].os == d[iy].n * vn && d[ix].is == vn) {
	  nx = d[iy].n;
	  ny = d[ix].n;
     }
     else
	  return 0;

     ntot = vn * FFTW(mpi_local_size_2d_transposed)(nx, ny, MPI_COMM_WORLD,
						    &local_ni, &local_starti,
						    &local_no, &local_starto);
     ni = nx;
     nresti = ny * vn;
     no = ny;
     nresto = nx * vn;
     alloc_local(ntot, p->in == p->out);

     pln = FFTW(mpi_plan_many_transpose)(nx, ny, vn,
					 FFTW_MPI_DEFAULT_BLOCK,
					 FFTW_MPI_DEFAULT_BLOCK,
					 local_in, local_out,
					 MPI_COMM_WORLD, flags);
     
     if (pln) make_scramblers(2, nx, ny, vn, flags | FFTW_MPI_TRANSPOSED);
     
#if 0
     if (pln && vn == 1) {
	  int i, j;
	  bench_real *ri = (bench_real *) p->in;
	  bench_real *ro = (bench_real *) p->out;
	  if (!ri || !ro) return pln;
	  for (i = 0; i < nx * ny; ++i)
	       ri[i] = i;
	  after_problem_rcopy_from(p, ri);
	  FFTW(execute)(pln);
	  after_problem_rcopy_to(p, ro);
	  if (my_pe == 0) {
	       for (i = 0; i < nx; ++i) {
		    for (j = 0; j < ny; ++j)
			 printf("  %3g", ro[j * nx + i]);
		    printf("\n");
	       }
	  }
     }
#endif

     return pln;
}

static FFTW(plan) mkplan_r2r(bench_problem *p, int flags)
{
     if ((p->sz->rnk == 0 || (p->sz->rnk == 1 && p->sz->dims[0].n == 1))
	 && p->vecsz->rnk >= 2 && p->vecsz->rnk <= 3)
	  return mkplan_transpose(p, flags);

     return 0;
}

FFTW(plan) mkplan(bench_problem *p, int flags)
{
     switch (p->kind) {
         case PROBLEM_COMPLEX: return mkplan_complex(p, flags);
         case PROBLEM_REAL: return mkplan_real(p, flags);
         case PROBLEM_R2R: return mkplan_r2r(p, flags);
         default: BENCH_ASSERT(0);
     }
     return 0;
}

void main_init(int *argc, char ***argv)
{
     MPI_Init(argc, argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &my_pe);
     MPI_Comm_size(MPI_COMM_WORLD, &n_pes);
     if (my_pe != 0) verbose = -999;
     recv_cnt = (int *) bench_malloc(sizeof(int) * n_pes);
     recv_off = (int *) bench_malloc(sizeof(int) * n_pes);
}

void final_cleanup(void)
{
     alloc_local(0, 0);
     bench_free(recv_off); recv_off = 0;
     bench_free(recv_cnt); recv_cnt = 0;
     FFTW(destroy_plan)(plan_scramble_in); plan_scramble_in = 0;
     FFTW(destroy_plan)(plan_unscramble_out); plan_unscramble_out = 0;
     MPI_Finalize();
}

void bench_exit(int status)
{
     MPI_Abort(MPI_COMM_WORLD, status);
}
