/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* transposes of unit-stride arrays, including arrays of N-tuples and
   non-square matrices, using cache-oblivious recursive algorithms */

#include "ifftw.h"
#include <string.h> /* memcpy */

#define CUTOFF 100 /* size below which we do a naive transpose */

/*************************************************************************/
/* some utilities for the solvers */

static int Ntuple_transposable(const iodim *a, const iodim *b,
			       int vl, int s, R *ri, R *ii)
{
     return(2 == s && (ii == ri + 1 || ri == ii + 1)
	    &&
	    ((a->is == b->os && a->is == (vl*2)
	      && a->os == b->n * (vl*2) && b->is == a->n * (vl*2))
	     ||
	     (a->os == b->is && a->os == (vl*2)
	      && a->is == b->n * (vl*2) && b->os == a->n * (vl*2))));
}


/* our solvers' transpose routines work for square matrices of arbitrary
   stride, or for non-square matrices of a given vl*vl2 corresponding
   to the N of the Ntuple with vl2 == s. */
int X(transposable)(const iodim *a, const iodim *b,
		    int vl, int s, R *ri, R *ii)
{
     return ((a->n == b->n && a->os == b->is && a->is == b->os)
	     || Ntuple_transposable(a, b, vl, s, ri, ii));
}

int gcd(int a, int b)
{
     int r;
     do {
	  r = a % b;
	  a = b;
	  b = r;
     } while (r != 0);
     
     return a;
}

/* all of the solvers need to extract n, m, d, n/d, and m/d from the
   two iodims, so we put it here to save code space */
void X(transpose_dims)(const iodim *a, const iodim *b,
		       int *n, int *m, int *d, int *nd, int *md)
{
     int n0, m0, d0;
     /* matrix should be n x m, row-major */
     if (a->is < b->is) {
	  *n = n0 = b->n;
	  *m = m0 = a->n;
     }
     else {
	  *n = n0 = a->n;
	  *m = m0 = b->n;
     }
     *d = d0 = gcd(n0, m0);
     *nd = n0 / d0;
     *md = m0 / d0;
}

/* use the simple square transpose in the solver for square matrices
   that aren't too big or which have the wrong stride */
int X(transpose_simplep)(const iodim *a, const iodim *b, int vl, int s,
			 R *ri, R *ii)
{
     return (a->n == b->n &&
	     (a->n*(vl*2) < CUTOFF 
	      ||  !Ntuple_transposable(a, b, vl, s, ri, ii)));
}

/* use the slow general transpose if the buffer would be more than 1/8
   the whole transpose and the transpose is fairly big.
   (FIXME: use the CONSERVE_MEMORY flag?) */
int X(transpose_slowp)(const iodim *a, const iodim *b, int N)
{
     int d = gcd(a->n, b->n);
     return (d < 8 && (a->n * b->n * N) / d > 65536);
}

/*************************************************************************/
/* Out-of-place transposes: */

/* Transpose A (n x m) to B (m x n), where A and B are stored
   as n x fda and m x fda arrays, respectively, operating on N-tuples: */
static void rec_transpose_Ntuple(R *A, R *B, int n, int m, int fda, int fdb,
			  int N)
{
     if (n == 1 || m == 1 || (n + m) * N < CUTOFF*2) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < m; ++j) {
		    for (k = 0; k < N; ++k) { /* FIXME: unroll */
			 B[(j*fdb + i) * N + k] = A[(i*fda + j) * N + k];
		    }
	       }
	  }
     }
     else if (n > m) {
	  int n2 = n / 2;
	  rec_transpose_Ntuple(A, B, n2, m, fda, fdb, N);
	  rec_transpose_Ntuple(A + n2*N*fda, B + n2*N, n - n2, m, fda, fdb, N);
     }
     else {
	  int m2 = m / 2;
	  rec_transpose_Ntuple(A, B, n, m2, fda, fdb, N);
	  rec_transpose_Ntuple(A + m2*N, B + m2*N*fdb, n, m - m2, fda, fdb, N);
     }
}

/*************************************************************************/
/* In-place transposes of square matrices of N-tuples: */

/* Transpose both A and B, where A is n x m and B is m x n, storing
   the transpose of A in B and the transpose of B in A.  A and B
   are actually stored as n x fda and m x fda arrays. */
static void rec_transpose_swap_Ntuple(R *A, R *B, int n, int m, int fda, int N)
{
     if (n == 1 || m == 1 || (n + m) * N <= CUTOFF*2) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < m; ++j) {
		    for (k = 0; k < N; ++k) { /* FIXME: unroll */
			 R a = A[(i*fda + j) * N + k];
			 A[(i*fda + j) * N + k] = B[(j*fda + i) * N + k];
			 B[(j*fda + i) * N + k] = a;
		    }
	       }
	  }
     }
     else if (n > m) {
	  int n2 = n / 2;
	  rec_transpose_swap_Ntuple(A, B, n2, m, fda, N);
	  rec_transpose_swap_Ntuple(A + n2*N*fda, B + n2*N, n - n2, m, fda, N);
     }
     else {
	  int m2 = m / 2;
	  rec_transpose_swap_Ntuple(A, B, n, m2, fda, N);
	  rec_transpose_swap_Ntuple(A + m2*N, B + m2*N*fda, n, m - m2, fda, N);
     }
}

/* Transpose A, an n x n matrix (stored as n x fda), in-place. */
static void rec_transpose_sq_ip_Ntuple(R *A, int n, int fda, int N)
{
     if (n == 1)
	  return;
     else if (n*N <= CUTOFF) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = i + 1; j < n; ++j) {
		    for (k = 0; k < N; ++k) { /* FIXME: unroll */
			 R a = A[(i*fda + j) * N + k];
			 A[(i*fda + j) * N + k] = A[(j*fda + i) * N + k];
			 A[(j*fda + i) * N + k] = a;
		    }
	       }
	  }
     }
     else {
	  int n2 = n / 2;
	  rec_transpose_sq_ip_Ntuple(A, n2, fda, N);
	  rec_transpose_sq_ip_Ntuple((A + n2*N) + n2*N*fda, n - n2, fda, N);
	  rec_transpose_swap_Ntuple(A + n2*N, A + n2*N*fda, n2, n - n2, fda,N);
     }
}

/*************************************************************************/
/* In-place transposes of non-square matrices: */

/* Transpose the matrix A in-place, where A is an (n*d) x (m*d) matrix
   of N-tuples and buf contains at least n*m*d*N elements.  In
   general, to transpose a p x q matrix, you should call this routine
   with d = gcd(p, q), n = p/d, and m = q/d. */
void X(transpose)(R *A, int n, int m, int d, int N, R *buf)
{
     A(n > 0 && m > 0 && N > 0 && d > 0);
     if (d == 1) {
	  rec_transpose_Ntuple(A, buf, n,m, m,n, N);
	  memcpy(A, buf, m*n*N*sizeof(R));
     }
     else if (n*m == 1) {
	  rec_transpose_sq_ip_Ntuple(A, d, d, N);
     }
     else {
	  int i, num_el = n*m*d*N;

	  /* treat as (d x n) x (d' x m) matrix.  (d' = d) */

	  /* First, transpose d x (n x d') x m to d x (d' x n) x m,
	     using the buf matrix.  This consists of d transposes
	     of contiguous n x d' matrices of m-tuples. */
	  if (n > 1) {
	       for (i = 0; i < d; ++i) {
		    rec_transpose_Ntuple(A + i*num_el, buf,
					 n,d, d,n, m*N);
		    memcpy(A + i*num_el, buf, num_el*sizeof(R));
	       }
	  }
	  
	  /* Now, transpose (d x d') x (n x m) to (d' x d) x (n x m), which
	     is a square in-place transpose of n*m-tuples: */
	  rec_transpose_sq_ip_Ntuple(A, d, d, n*m*N);

	  /* Finally, transpose d' x ((d x n) x m) to d' x (m x (d x n)),
	     using the buf matrix.  This consists of d' transposes
	     of contiguous d*n x m matrices. */
	  if (m > 1) {
	       for (i = 0; i < d; ++i) {
		    rec_transpose_Ntuple(A + i*num_el, buf,
					 d*n,m, m,d*n, N);
		    memcpy(A + i*num_el, buf, num_el*sizeof(R));
	       }
	  }
     }
}

/*************************************************************************/
/* In-place transpose routine from TOMS.  This routine is much slower
   than the cache-oblivious algorithm above, but is has the advantage
   of requiring less buffer space for the case of gcd(nx,ny) small. */

/*
 * TOMS Transpose.  Revised version of algorithm 380.
 * 
 * These routines do in-place transposes of arrays.
 * 
 * [ Cate, E.G. and Twigg, D.W., ACM Transactions on Mathematical Software, 
 *   vol. 3, no. 1, 104-110 (1977) ]
 * 
 * C version by Steven G. Johnson. February 1997.
 */

/*
 * "a" is a 1D array of length ny*nx*N which constains the nx x ny
 * matrix of N-tuples to be transposed.  "a" is stored in row-major
 * order (last index varies fastest).  move is a 1D array of length
 * move_size used to store information to speed up the process.  The
 * value move_size=(ny+nx)/2 is recommended.  buf should be an array
 * of length 2*N.
 * 
 */

void X(transpose_slow)(R *a, int nx, int ny, int N,
		       char *move, int move_size, R *buf)
{
     int i, j, im, mn;
     R *b, *c, *d;
     int ncount;
     int k;
     
     /* check arguments and initialize: */
     A(ny > 0 && nx > 0 && N > 0 && move_size > 0);
     
     b = buf;
     
     if (ny == nx) {
	  /*
	   * if matrix is square, exchange elements a(i,j) and a(j,i):
	   */
	  for (i = 0; i < nx; ++i)
	       for (j = i + 1; j < nx; ++j) {
		    memcpy(b, &a[N * (i + j * nx)], N * sizeof(R));
		    memcpy(&a[N * (i + j * nx)], &a[N * (j + i * nx)], N * sizeof(R));
		    memcpy(&a[N * (j + i * nx)], b, N * sizeof(R));
	       }
	  return;
     }
     c = buf + N;
     ncount = 2;		/* always at least 2 fixed points */
     k = (mn = ny * nx) - 1;
     
     for (i = 0; i < move_size; ++i)
	  move[i] = 0;
     
     if (ny >= 3 && nx >= 3)
	  ncount += gcd(ny - 1, nx - 1) - 1;	/* # fixed points */
     
     i = 1;
     im = ny;
     
     while (1) {
	  int i1, i2, i1c, i2c;
	  int kmi;
	  
	  /** Rearrange the elements of a loop
	      and its companion loop: **/
	  
	  i1 = i;
	  kmi = k - i;
	  memcpy(b, &a[N * i1], N * sizeof(R));
	  i1c = kmi;
	  memcpy(c, &a[N * i1c], N * sizeof(R));
	  
	  while (1) {
	       i2 = ny * i1 - k * (i1 / nx);
	       i2c = k - i2;
	       if (i1 < move_size)
		    move[i1] = 1;
	       if (i1c < move_size)
		    move[i1c] = 1;
	       ncount += 2;
	       if (i2 == i)
		    break;
	       if (i2 == kmi) {
		    d = b;
		    b = c;
		    c = d;
		    break;
	       }
	       memcpy(&a[N * i1], &a[N * i2], 
		      N * sizeof(R));
	       memcpy(&a[N * i1c], &a[N * i2c], 
		      N * sizeof(R));
	       i1 = i2;
	       i1c = i2c;
	  }
	  memcpy(&a[N * i1], b, N * sizeof(R));
	  memcpy(&a[N * i1c], c, N * sizeof(R));
	  
	  if (ncount >= mn)
	       break;	/* we've moved all elements */
	  
	  /** Search for loops to rearrange: **/
	  
	  while (1) {
	       int max = k - i;
	       ++i;
	       A(i <= max);
	       im += ny;
	       if (im > k)
		    im -= k;
	       i2 = im;
	       if (i == i2)
		    continue;
	       if (i >= move_size) {
		    while (i2 > i && i2 < max) {
			 i1 = i2;
			 i2 = ny * i1 - k * (i1 / nx);
		    }
		    if (i2 == i)
			 break;
	       } else if (!move[i])
		    break;
	  }
     }
}
