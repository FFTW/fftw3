#region Copyright and License
/*
This file is part of FFTW.NET, a wrapper around the FFTW library
for the .NET framework.
Copyright (C) 2017 Tobias Meyer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#endregion

using System;
using System.Numerics;

namespace FFTW.NET
{
	public sealed class FftwPlanRC : FftwPlan<double, Complex>
	{
		public Array<double> BufferReal => Buffer1;
		public Array<Complex> BufferComplex => Buffer2;

		FftwPlanRC(Array<double> bufferReal, Array<Complex> bufferComplex, int rank, int[] n, bool verifyRankAndSize, DftDirection direction, PlannerFlags plannerFlags, int nThreads)
			: base(bufferReal, bufferComplex, rank, n, verifyRankAndSize, direction, plannerFlags, nThreads) { }

		protected override IntPtr GetPlan(int rank, int[] n, IntPtr bufferReal, IntPtr bufferComplex, DftDirection direction, PlannerFlags plannerFlags)
		{
			if (direction == DftDirection.Forwards)
				return FftwInterop.fftw_plan_dft_r2c(rank, n, bufferReal, bufferComplex, plannerFlags);
			else
				return FftwInterop.fftw_plan_dft_c2r(rank, n, bufferComplex, bufferReal, plannerFlags);
		}

		protected override void VerifyRankAndSize(Array<double> bufferReal, Array<Complex> bufferComplex)
		{
			if (bufferReal.Rank != bufferComplex.Rank)
				throw new ArgumentException($"{nameof(bufferReal)} and {nameof(bufferComplex)} must have the same rank and size.");
			for (int i = 0; i < bufferReal.Rank-1; i++)
			{
				if (bufferReal.GetLength(i) != bufferComplex.GetLength(i))
					throw new ArgumentException($"Lenght of {nameof(bufferComplex)} must be equal to n[0]*...*(n[n.Length - 1] / 2 + 1) with n being the size of {nameof(bufferReal)}.");
			}
			if (bufferReal.GetLength(bufferReal.Rank - 1) / 2 + 1 != bufferComplex.GetLength(bufferReal.Rank - 1))
				throw new ArgumentException($"Lenght of {nameof(bufferComplex)} must be equal to n[0]*...*(n[n.Length - 1] / 2 + 1) with n being the size of {nameof(bufferReal)}.");
		}

		protected override void VerifyMinSize(Array<double> bufferReal, Array<Complex> bufferComplex, int[] n)
		{
			long sizeReal = 1;
			for (int i = 0; i < n.Length - 1; i++)
				sizeReal *= n[i];
			long sizeComplex = sizeReal * (n[n.Length - 1] / 2 + 1);
			sizeReal *= n[n.Length - 1];

			if (bufferReal.LongLength < sizeReal)
				throw new ArgumentException($"{nameof(bufferReal)} is too small.");

			if (bufferComplex.LongLength < sizeComplex)
				throw new ArgumentException($"{nameof(bufferComplex)} is too small.");
		}

		/// <summary>
		/// Initializes a new plan using the provided input and output buffers.
		/// These buffers may be overwritten during initialization.
		/// </summary>
		public static FftwPlanRC Create(Array<double> bufferReal, Array<Complex> bufferComplex, DftDirection direction,  PlannerFlags plannerFlags = PlannerFlags.Default, int nThreads = 1)
		{
			FftwPlanRC plan = new FftwPlanRC(bufferReal, bufferComplex, bufferReal.Rank, bufferReal.GetSize(), true, direction, plannerFlags, nThreads);
			if (plan.IsZero)
				return null;
			return plan;
		}
		
		internal static FftwPlanRC Create(Array<double> bufferReal, Array<Complex> bufferComplex, int rank, int[] n, DftDirection direction, PlannerFlags plannerFlags, int nThreads)
		{
			FftwPlanRC plan = new FftwPlanRC(bufferReal, bufferComplex, rank, n, false, direction, plannerFlags, nThreads);
			if (plan.IsZero)
				return null;
			return plan;
		}
	}
}