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

#include "bench.h"

/* FFT routine used for calibratin the timer */

#define WS(a, b) a * b

void bench_fft8(const float *ri, const float *ii, float *ro, float *io,
		int is, int os)
{
     const float KP707106781 = 0.707106781186547524400844362104849039284835938;
     float T3, Tn, Ti, TC, T6, TB, Tl, To, Td, TN, Tz, TH, Ta, TM, Tu;
     float TG;
     {
	  float T1, T2, Tj, Tk;
	  T1 = ri[0];
	  T2 = ri[WS(is, 4)];
	  T3 = T1 + T2;
	  Tn = T1 - T2;
	  {
	       float Tg, Th, T4, T5;
	       Tg = ii[0];
	       Th = ii[WS(is, 4)];
	       Ti = Tg + Th;
	       TC = Tg - Th;
	       T4 = ri[WS(is, 2)];
	       T5 = ri[WS(is, 6)];
	       T6 = T4 + T5;
	       TB = T4 - T5;
	  }
	  Tj = ii[WS(is, 2)];
	  Tk = ii[WS(is, 6)];
	  Tl = Tj + Tk;
	  To = Tj - Tk;
	  {
	       float Tb, Tc, Tv, Tw, Tx, Ty;
	       Tb = ri[WS(is, 7)];
	       Tc = ri[WS(is, 3)];
	       Tv = Tb - Tc;
	       Tw = ii[WS(is, 7)];
	       Tx = ii[WS(is, 3)];
	       Ty = Tw - Tx;
	       Td = Tb + Tc;
	       TN = Tw + Tx;
	       Tz = Tv - Ty;
	       TH = Tv + Ty;
	  }
	  {
	       float T8, T9, Tq, Tr, Ts, Tt;
	       T8 = ri[WS(is, 1)];
	       T9 = ri[WS(is, 5)];
	       Tq = T8 - T9;
	       Tr = ii[WS(is, 1)];
	       Ts = ii[WS(is, 5)];
	       Tt = Tr - Ts;
	       Ta = T8 + T9;
	       TM = Tr + Ts;
	       Tu = Tq + Tt;
	       TG = Tt - Tq;
	  }
     }
     {
	  float T7, Te, TP, TQ;
	  T7 = T3 + T6;
	  Te = Ta + Td;
	  ro[WS(os, 4)] = T7 - Te;
	  ro[0] = T7 + Te;
	  TP = Ti + Tl;
	  TQ = TM + TN;
	  io[WS(os, 4)] = TP - TQ;
	  io[0] = TP + TQ;
     }
     {
	  float Tf, Tm, TL, TO;
	  Tf = Td - Ta;
	  Tm = Ti - Tl;
	  io[WS(os, 2)] = Tf + Tm;
	  io[WS(os, 6)] = Tm - Tf;
	  TL = T3 - T6;
	  TO = TM - TN;
	  ro[WS(os, 6)] = TL - TO;
	  ro[WS(os, 2)] = TL + TO;
     }
     {
	  float Tp, TA, TJ, TK;
	  Tp = Tn + To;
	  TA = KP707106781 * (Tu + Tz);
	  ro[WS(os, 5)] = Tp - TA;
	  ro[WS(os, 1)] = Tp + TA;
	  TJ = TC - TB;
	  TK = KP707106781 * (TG + TH);
	  io[WS(os, 5)] = TJ - TK;
	  io[WS(os, 1)] = TJ + TK;
     }
     {
	  float TD, TE, TF, TI;
	  TD = TB + TC;
	  TE = KP707106781 * (Tz - Tu);
	  io[WS(os, 7)] = TD - TE;
	  io[WS(os, 3)] = TD + TE;
	  TF = Tn - To;
	  TI = KP707106781 * (TG - TH);
	  ro[WS(os, 7)] = TF - TI;
	  ro[WS(os, 3)] = TF + TI;
     }
}
