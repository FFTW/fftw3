/* fftw hook to be used in the benchmark program.  
   
   We keep it in a separate file because 

   1) bench.c is supposed to test the API---we do not want to #include
      "ifftw.h" and accidentally use internal symbols/macros.
   2) this code is a royal mess.  The messiness is due to
      A) confusion between internal fftw tensors and bench_tensor's
         (which we want to keep separate because the benchmark
	  program tests other routines too)
      B) despite A), our desire to recycle the libbench verifier.
*/

#include <stdio.h>
#include "bench-user.h"
#include "api.h"
#include "dft.h"

extern int paranoid; /* in bench.c */


/* Make a copy of the size tensor, with the same dimensions, but with
   the strides corresponding to a "packed" row-major array with the
   given stride. */
static tensor *pack_tensor(const tensor *sz, int s)
{
     tensor *x = X(tensor_copy)(sz);
     if (FINITE_RNK(x->rnk) && x->rnk > 0) {
	  int i;
	  x->dims[x->rnk - 1].is = s;
	  x->dims[x->rnk - 1].os = s;
	  for (i = x->rnk - 1; i > 0; --i) {
	       x->dims[i - 1].is = x->dims[i].is * x->dims[i].n;
	       x->dims[i - 1].os = x->dims[i].os * x->dims[i].n;
	  }
     }
     return x;
}

/*
  transform a fftw tensor into a bench_tensor.
*/
static bench_tensor *fftw_tensor_to_bench_tensor(tensor *t)
{
     bench_tensor *bt = mktensor(t->rnk);

     if (FINITE_RNK(t->rnk)) {
	  int i;
	  for (i = 0; i < t->rnk; ++i) {
	       bt->dims[i].n = t->dims[i].n;
	       bt->dims[i].is = t->dims[i].is;
	       bt->dims[i].os = t->dims[i].os;
	  }
     }
     return bt;
}

/*
  dotens2: 
*/
typedef struct dotens2_closure_s {
     void (*apply)(struct dotens2_closure_s *k, 
		   int indx0, int ondx0, int indx1, int ondx1);
} dotens2_closure;

static void recur(int rnk, const iodim *dims0, const iodim *dims1,
		  dotens2_closure *k, 
		  int indx0, int ondx0, int indx1, int ondx1)
{
     if (rnk == 0)
          k->apply(k, indx0, ondx0, indx1, ondx1);
     else {
          int i, n = dims0[0].n;
          int is0 = dims0[0].is;
          int os0 = dims0[0].os;
          int is1 = dims1[0].is;
          int os1 = dims1[0].os;

	  A(n == dims1[0].n);

          for (i = 0; i < n; ++i) {
               recur(rnk - 1, dims0 + 1, dims1 + 1, k,
		     indx0, ondx0, indx1, ondx1);
	       indx0 += is0; ondx0 += os0;
	       indx1 += is1; ondx1 += os1;
	  }
     }
}

void dotens2(const tensor *sz0, const tensor *sz1, dotens2_closure *k)
{
     A(sz0->rnk == sz1->rnk);
     if (sz0->rnk == RNK_MINFTY)
          return;
     recur(sz0->rnk, sz0->dims, sz1->dims, k, 0, 0, 0, 0);
}


/**************************************************************/
/* DFT verifier: */
/* copy A into B, using output stride of A and input stride of B */
typedef struct {
     dotens2_closure k;
     R *ra; R *ia;
     R *rb; R *ib;
} dftcpy_closure;

static void dftcpy0(dotens2_closure *k_, 
		    int indxa, int ondxa, int indxb, int ondxb)
{
     dftcpy_closure *k = (dftcpy_closure *)k_;
     k->rb[indxb] = k->ra[ondxa];
     k->ib[indxb] = k->ia[ondxa];
     UNUSED(indxa); UNUSED(ondxb);
}

static void dftcpy(R *ra, R *ia, const tensor *sza, 
		   R *rb, R *ib, const tensor *szb)
{
     dftcpy_closure k;
     k.k.apply = dftcpy0;
     k.ra = ra; k.ia = ia; k.rb = rb; k.ib = ib;
     dotens2(sza, szb, &k.k);
}

typedef struct {
     dofft_closure k;
     const problem_dft *p;
     plan *pln;
} dofft_dft_closure;

static void dft_apply(dofft_closure *k_, bench_complex *in, bench_complex *out)
{
     dofft_dft_closure *k = (dofft_dft_closure *)k_;
     const problem_dft *p = k->p;
     tensor *totalsz, *pckdsz;

     totalsz = X(tensor_append)(p->vecsz, p->sz);
     pckdsz = pack_tensor(totalsz, 2);

     dftcpy(&c_re(in[0]), &c_im(in[0]), pckdsz, p->ri, p->ii, totalsz);
     k->pln->adt->solve(k->pln, &(k->p->super));
     dftcpy(p->ro, p->io, totalsz, &c_re(out[0]), &c_im(out[0]), pckdsz);

     X(tensor_destroy)(totalsz);
     X(tensor_destroy)(pckdsz);
}

static void dodft(plan *pln, const problem_dft *p, int rounds, double tol)
{
     dofft_dft_closure k;
     errors e;
     bench_tensor *bsz = fftw_tensor_to_bench_tensor(p->sz);
     bench_tensor *bvecsz = fftw_tensor_to_bench_tensor(p->vecsz);

     k.k.apply = dft_apply; k.p = p; k.pln = pln;
     verify_dft((dofft_closure *)&k, bsz, bvecsz, FFT_SIGN, rounds, tol, &e);

     tensor_destroy(bsz);
     tensor_destroy(bvecsz);
}

/* end dft verifier */
/**************************************************************/

static void hook(plan *pln, const problem *p_, int optimalp)
{
     int rounds = 5;
     double tol = SINGLE_PRECISION ? 1.0e-3 : 1.0e-10;
     UNUSED(optimalp);

     if (verbose > 5) {
	  printer *pr = X(mkprinter_file)(stdout);
	  pr->print(pr, "%P:%(%p%)\n", p_, pln);
	  X(printer_destroy)(pr);
	  printf("cost %g  \n\n", pln->pcost);
     }

     if (paranoid) {
	  AWAKE(pln, 1);
	  if (DFTP(p_))
	       dodft(pln, (const problem_dft *) p_, rounds, tol);
	  else {
	       /* TODO */
	  }
	  AWAKE(pln, 0);
     }
}

void install_hook(void)
{
     planner *plnr = X(the_planner)();
     plnr->hook = hook;
}
