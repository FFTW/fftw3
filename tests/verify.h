/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

#include "ifftw.h"

typedef struct {
     R r;
     R i;
} C;

#define arand X(arand)
void arand(C *a, uint n);

#define mkreal X(mkreal)
void mkreal(C *A, uint n);

#define mkhermitian X(mkhermitian)
void mkhermitian(C *A, uint rank, const iodim *dim);

#define aadd X(aadd)
void aadd(C *c, C *a, C *b, uint n);

#define asub X(asub)
void asub(C *c, C *a, C *b, uint n);

#define arol X(arol)
void arol(C *b, C *a, uint n, uint nb, uint na);

#define aphase_shift X(aphase_shift)
void aphase_shift(C *b, C *a, uint n, uint nb, uint na, double sign);

#define ascale X(ascale)
void ascale(C *a, C alpha, uint n);

#define acmp X(acmp)
double acmp(C *a, C *b, uint n, const char *test, double tol);

#define mydrand X(mydrand)
double mydrand(void);

#define impulse X(impulse)
void impulse(void (*dofft)(void *nfo, C *in, C *out),
		  void *nfo, 
		  uint n, uint vecn, 
		  C *inA, C *inB, C *inC,
		  C *outA, C *outB, C *outC,
		  C *tmp, uint rounds, double tol);

#define linear X(linear)
void linear(void (*dofft)(void *nfo, C *in, C *out),
		    void *nfo, int realp,
		    uint n, C *inA, C *inB, C *inC, C *outA,
		    C *outB, C *outC, C *tmp, uint rounds, double tol);

enum { TIME_SHIFT, FREQ_SHIFT };

#define tf_shift X(tf_shift)
void tf_shift(void (*dofft)(void *nfo, C *in, C *out),
	      void *nfo, int realp, const tensor *sz,
	      uint n, uint vecn,
	      C *inA, C *inB, C *outA, C *outB, C *tmp,
	      uint rounds, double tol, int which_shift);


#define verify_pack X(verify_pack)
tensor *verify_pack(const tensor *sz, int s);
