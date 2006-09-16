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

static ptrdiff_t local_nx, local_x_start, ny, local_ny, local_y_start, nx, xblock, yblock;
bench_real *local_in = 0, *local_out = 0;

int n_pes = 1, my_pe = 0;
int *recv_cnt = 0, *recv_off; /* for MPI_Gatherv */

static void alloc_local(ptrdiff_t nreal, int inplace)
{
     fftw_free(local_in);
     if (local_out != local_in) fftw_free(local_out);
     if (nreal > 0) {
	  ptrdiff_t i;
	  local_in = (bench_real*) fftw_malloc(nreal * sizeof(bench_real));
	  if (inplace)
	       local_out = local_in;
	  else
	       local_out = (bench_real*) fftw_malloc(nreal * sizeof(bench_real));
	  for (i = 0; i < nreal; ++i) local_in[i] = 0.0;
     }
}

void after_problem_rcopy_from(bench_problem *p, bench_real *ri)
{
     ptrdiff_t i, j;
     UNUSED(p);
     for (i = 0; i < local_nx; ++i)
	  for (j = 0; j < ny; ++j)
	       local_in[i*ny + j] = ri[(i+local_x_start)*ny + j];
}

static void collect_data(ptrdiff_t block, ptrdiff_t n, ptrdiff_t vn,
			 bench_real *out)
{
     int i, tot = 0;
     for (i = 0; i < n / block; ++i)
	  recv_cnt[i] = block * vn;
     if (i < n_pes)
	  recv_cnt[i++] = (n - block * i) * vn;
     for (; i < n_pes; ++i)
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
     collect_data(yblock, ny, nx, ro);
}

static FFTW(plan) mkplan_complex(bench_problem *p, int flags)
{
     return 0;
}

static FFTW(plan) mkplan_real(bench_problem *p, int flags)
{
     return 0;
}

static FFTW(plan) mkplan_transpose(bench_problem *p, int flags)
{
     ptrdiff_t ntot;
     int vn, ix, iy, i;
     const bench_iodim *d = p->vecsz->dims;

     if (p->vecsz->rnk == 3) {
	  for (i = 0; i < 3; ++i)
	       if (d[i].is == 1 && d[i].is == 1) {
		    vn = d[0].n;
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

     ntot = vn * FFTW(mpi_local_size_2d_transposed)(nx, ny, 
						    FFTW_MPI_DEFAULT_BLOCK,
						    FFTW_MPI_DEFAULT_BLOCK,
						    MPI_COMM_WORLD,
						    &local_nx, &local_x_start,
						    &local_ny, &local_y_start);
     xblock = (nx + n_pes - 1) / n_pes;
     yblock = (ny + n_pes - 1) / n_pes;
     alloc_local(ntot, p->in == p->out);

     return FFTW(mpi_plan_many_transpose)(nx, ny, vn,
					  FFTW_MPI_DEFAULT_BLOCK,
					  FFTW_MPI_DEFAULT_BLOCK,
					  local_in, local_out,
					  MPI_COMM_WORLD, flags);

     nx *= vn;
     ny *= vn;
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
         case PROBLEM_COMPLEX:    return mkplan_complex(p, flags);
         case PROBLEM_REAL:       return mkplan_real(p, flags);
         case PROBLEM_R2R:        return mkplan_r2r(p, flags);
         default: BENCH_ASSERT(0); return 0;
     }
}

void main_init(int *argc, char ***argv)
{
     MPI_Init(argc, argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &my_pe);
     MPI_Comm_size(MPI_COMM_WORLD, &n_pes);
     if (my_pe != 0) verbose = -999;
     recv_cnt = (int *) fftw_malloc(sizeof(int) * n_pes);
     recv_off = (int *) fftw_malloc(sizeof(int) * n_pes);
}

void final_cleanup(void)
{
     alloc_local(0, 0);
     fftw_free(recv_off);
     fftw_free(recv_cnt);
     MPI_Finalize();
}

void bench_exit(int status)
{
     MPI_Abort(MPI_COMM_WORLD, status);
}
