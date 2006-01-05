/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Massachusetts Institute of Technology
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

/* $Id: main.c,v 1.4 2006-01-05 03:04:27 stevenj Exp $ */

#include "bench.h"

/* On some systems, we are required to define a dummy main-like
   routine (called "MAIN__" or something similar in order to link a C
   main() with the Fortran libraries).  This is detected by autoconf;
   see the autoconf 2.52 or later manual. */
#ifdef F77_DUMMY_MAIN
#  ifdef __cplusplus
     extern "C"
#  endif
     int F77_DUMMY_MAIN() { return 1; }
#endif

/* in a separate file so that the user can override it */
int main(int argc, char *argv[])
{
     return aligned_main(argc, argv);
}
