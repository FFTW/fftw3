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

/* disable non-ntuples for now, since we don't need real transposes */
#define NON_NTUPLE 0 /* include specialized code for non-Ntuples */

/*************************************************************************/
/* some utilities for the solvers */

/* our solvers' transpose routines work for square matrices of arbitrary
   stride, or for non-square matrices of a given vl*vl2 corresponding
   to the N of the Ntuple with vl2 == s. */
int X(transposable)(const iodim *a, const iodim *b, int vl, int vl2, int s)
{
     return ((a->n == b->n && a->os == b->is && a->is == b->os)
	     ||
	     (vl2 == s
	      &&
	      ((a->is == b->os && a->is == (vl*vl2)
		&& a->os == b->n * (vl*vl2) && b->is == a->n * (vl*vl2))
	       ||
	       (a->os == b->is && a->os == (vl*vl2)
		&& a->is == b->n * (vl*vl2) && b->os == a->n * (vl*vl2)))));
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
int X(transpose_simplep)(const iodim *a, const iodim *b, int N)
{
     return (a->n == b->n && (a->n*N < CUTOFF ||  X(imin)(a->is, a->os) != N));
}

/* use the slow general transpose if the buffer would be more than 1/8
   the whole transpose and the transpose is fairly big. */
int X(transpose_slowp)(const iodim *a, const iodim *b, int N)
{
     int d = gcd(a->n, b->n);
     return (d < 8 && (a->n * b->n * N) / d > 65536);
}

#if NON_NTUPLE
/*************************************************************************/
/* Out-of-place transposes: */

/* Transpose A (n x m) to B (m x n), where A and B are stored
   as n x fda and m x fda arrays, respectively. */
static void rec_transpose(R *A, R *B, int n, int m, int fda, int fdb)
{
     if (n + m < CUTOFF*2) {
	  int i, j;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < (m-3); j += 4) {
		    R a0, a1, a2, a3;
		    a0 = A[(i*fda + j)];
		    a1 = A[(i*fda + j) + 1];
		    a2 = A[(i*fda + j) + 2];
		    a3 = A[(i*fda + j) + 3];
		    B[j*fdb + i] = a0;
		    B[(j + 1)*fdb + i] = a1;
		    B[(j + 2)*fdb + i] = a2;
		    B[(j + 3)*fdb + i] = a3;
	       }
	       for (; j < m; ++j) {
		    B[j*fdb + i] = A[i*fda + j];
	       }
	  }
     }
     else if (n > m) {
	  int n2 = n / 2;
	  rec_transpose(A, B, n2, m, fda, fdb);
	  rec_transpose(A + n2*fda, B + n2, n - n2, m, fda, fdb);
     }
     else {
	  int m2 = m / 2;
	  rec_transpose(A, B, n, m2, fda, fdb);
	  rec_transpose(A + m2, B + m2*fdb, n, m - m2, fda, fdb);
     }
}

/*************************************************************************/
/* In-place transposes of square matrices: */

/* Transpose both A and B, where A is n x m and B is m x n, storing
   the transpose of A in B and the transpose of B in A.  A and B
   are actually stored as n x fda and m x fda arrays. */
static void rec_transpose_swap(R *A, R *B, int n, int m, int fda)
{
     if (n+m <= CUTOFF*2) {
	  int i, j;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < (m-3); j += 4) {
		    R a0, a1, a2, a3;
		    R b0, b1, b2, b3;
		    a0 = A[(i*fda + j)];
		    a1 = A[(i*fda + j) + 1];
		    a2 = A[(i*fda + j) + 2];
		    a3 = A[(i*fda + j) + 3];
		    b0 = B[j*fda + i];
		    b1 = B[(j + 1)*fda + i];
		    b2 = B[(j + 2)*fda + i];
		    b3 = B[(j + 3)*fda + i];
		    A[(i*fda + j)] = b0;
		    A[(i*fda + j) + 1] = b1;
		    A[(i*fda + j) + 2] = b2;
		    A[(i*fda + j) + 3] = b3;
		    B[j*fda + i] = a0;
		    B[(j + 1)*fda + i] = a1;
		    B[(j + 2)*fda + i] = a2;
		    B[(j + 3)*fda + i] = a3;
	       }
	       for (; j < m; ++j) {
		    R a = A[i*fda + j];
		    A[i*fda + j] = B[j*fda + i];
		    B[j*fda + i] = a;
	       }
	  }
     }
     else if (n > m) {
	  int n2 = n / 2;
	  rec_transpose_swap(A, B, n2, m, fda);
	  rec_transpose_swap(A + n2*fda, B + n2, n - n2, m, fda);
     }
     else {
	  int m2 = m / 2;
	  rec_transpose_swap(A, B, n, m2, fda);
	  rec_transpose_swap(A + m2, B + m2*fda, n, m - m2, fda);
     }
}

/* Transpose A, an n x n matrix (stored as n x fda), in-place. */
static void rec_transpose_sq_ip(R *A, int n, int fda)
{
     if (n <= CUTOFF) {
	  int i, j;
	  for (i = 0; i < n; ++i) {
	       for (j = i+1; j < (n-3); j += 4) {
		    R a0, a1, a2, a3;
		    R b0, b1, b2, b3;
		    a0 = A[(i*fda + j)];
		    a1 = A[(i*fda + j) + 1];
		    a2 = A[(i*fda + j) + 2];
		    a3 = A[(i*fda + j) + 3];
		    b0 = A[j*fda + i];
		    b1 = A[(j + 1)*fda + i];
		    b2 = A[(j + 2)*fda + i];
		    b3 = A[(j + 3)*fda + i];
		    A[(i*fda + j)] = b0;
		    A[(i*fda + j) + 1] = b1;
		    A[(i*fda + j) + 2] = b2;
		    A[(i*fda + j) + 3] = b3;
		    A[j*fda + i] = a0;
		    A[(j + 1)*fda + i] = a1;
		    A[(j + 2)*fda + i] = a2;
		    A[(j + 3)*fda + i] = a3;
	       }
	       for (; j < n; ++j) {
		    R a = A[i*fda + j];
		    A[i*fda + j] = A[j*fda + i];
		    A[j*fda + i] = a;
	       }
	  }
     }
     else {
	  int n2 = n / 2;
	  rec_transpose_sq_ip(A, n2, fda);
	  rec_transpose_sq_ip((A + n2) + fda*n2, n - n2, fda);
	  rec_transpose_swap(A + n2, A + fda*n2, n2, n - n2, fda);
     }
}

#endif /* NON_NTUPLE */

/*************************************************************************/
/* Out-of-place transposes: */

/* as rec_transpose, but operates on N-tuples: */
static void rec_transpose_Ntuple(R *A, R *B, int n, int m, int fda, int fdb,
			  int N)
{
     if (n == 1 || m == 1 || (n + m) * N < CUTOFF*2) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < m; ++j) {
		    for (k = 0; k < N; ++k) {
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

static void rec_transpose_swap_Ntuple(R *A, R *B, int n, int m, int fda, int N)
{
     if (n == 1 || m == 1 || (n + m) * N <= CUTOFF*2) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = 0; j < m; ++j) {
		    for (k = 0; k < N; ++k) {
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

static void rec_transpose_sq_ip_Ntuple(R *A, int n, int fda, int N)
{
     if (n == 1)
	  return;
     else if (n*N <= CUTOFF) {
	  int i, j, k;
	  for (i = 0; i < n; ++i) {
	       for (j = i + 1; j < n; ++j) {
		    for (k = 0; k < N; ++k) {
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
#if NON_NTUPLE
	  if (N == 1)
	       rec_transpose(A, buf, n,m, m,n);
	  else
#endif
	       rec_transpose_Ntuple(A, buf, n,m, m,n, N);
	  memcpy(A, buf, m*n*N*sizeof(R));
     }
     else if (n*m == 1) {
#if NON_NTUPLE
	  if (N == 1)
	       rec_transpose_sq_ip(A, d, d);
	  else
#endif
	       rec_transpose_sq_ip_Ntuple(A, d, d, N);
     }
     else {
	  int i, num_el = n*m*d*N;

	  /* treat as (d x n) x (d' x m) matrix.  (d' = d) */

	  /* First, transpose d x (n x d') x m to d x (d' x n) x m,
	     using the buf matrix.  This consists of d transposes
	     of contiguous n x d' matrices of m-tuples. */
	  if (n > 1) {
#if NON_NTUPLE
	       if (m*N == 1) {
		    for (i = 0; i < d; ++i) {
			 rec_transpose(A + i*num_el, buf, n,d, d,n);
			 memcpy(A + i*num_el, buf, num_el*sizeof(R));
		    }
	       }
	       else 
#endif
	       {
		    for (i = 0; i < d; ++i) {
			 rec_transpose_Ntuple(A + i*num_el, buf,
					      n,d, d,n, m*N);
			 memcpy(A + i*num_el, buf, num_el*sizeof(R));
		    }
	       }
	  }
	  
	  /* Now, transpose (d x d') x (n x m) to (d' x d) x (n x m), which
	     is a square in-place transpose of n*m-tuples: */
	  rec_transpose_sq_ip_Ntuple(A, d, d, n*m*N);

	  /* Finally, transpose d' x ((d x n) x m) to d' x (m x (d x n)),
	     using the buf matrix.  This consists of d' transposes
	     of contiguous d*n x m matrices. */
	  if (m > 1) {
#if NON_NTUPLE
	       if (N == 1) {
		    for (i = 0; i < d; ++i) {
			 rec_transpose(A + i*num_el, buf, d*n,m, m,d*n);
			 memcpy(A + i*num_el, buf, num_el*sizeof(R));
		    }
	       }
	       else 
#endif
	       {
		    for (i = 0; i < d; ++i) {
			 rec_transpose_Ntuple(A + i*num_el, buf,
					      d*n,m, m,d*n, N);
			 memcpy(A + i*num_el, buf, num_el*sizeof(R));
		    }
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
