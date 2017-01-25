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
	public abstract class FftwPlan<T1, T2> : IDisposable
		where T1 : struct
		where T2 : struct
	{
		IntPtr _plan = IntPtr.Zero;

		readonly Array<T1> _buffer1;
		readonly Array<T2> _buffer2;
		readonly PinnedGCHandle _pinIn;
		readonly PinnedGCHandle _pinOut;

		protected Array<T1> Buffer1 => _buffer1;
		protected Array<T2> Buffer2 => _buffer2;

		internal protected bool IsZero => _plan == IntPtr.Zero;

		internal protected FftwPlan(Array<T1> buffer1, Array<T2> buffer2, int rank, int[] n, bool verifyRankAndSize, DftDirection direction, PlannerFlags plannerFlags, int nThreads)
		{
			if (!FftwInterop.IsAvailable)
				throw new InvalidOperationException($"{nameof(FftwInterop.IsAvailable)} returns false.");

			if (verifyRankAndSize)
				VerifyRankAndSize(buffer1, buffer2);
			else
				VerifyMinSize(buffer1, buffer2, n);

			if (nThreads < 1)
				nThreads = Environment.ProcessorCount;

			_pinIn = PinnedGCHandle.Pin(buffer1.Buffer);
			_pinOut = PinnedGCHandle.Pin(buffer2.Buffer);
			_plan = IntPtr.Zero;

			lock (FftwInterop.Lock)
			{
				FftwInterop.fftw_plan_with_nthreads(nThreads);
				_plan = GetPlan(rank, n, _pinIn.Pointer, _pinOut.Pointer, direction, plannerFlags);
			}

			if (_plan == IntPtr.Zero)
			{
				_pinIn.Free();
				_pinOut.Free();
			}
		}

		protected abstract IntPtr GetPlan(int rank, int[] n, IntPtr input, IntPtr output, DftDirection direction, PlannerFlags plannerFlags);
		protected abstract void VerifyRankAndSize(Array<T1> input, Array<T2> output);
		protected abstract void VerifyMinSize(Array<T1> ipput, Array<T2> output, int[] n);


		public void Execute()
		{
			if (_plan == IntPtr.Zero)
				throw new ObjectDisposedException(this.GetType().FullName);

			FftwInterop.fftw_execute(_plan);
		}

		public void Dispose()
		{
			if (_plan != IntPtr.Zero)
			{
				FftwInterop.fftw_destroy_plan(_plan);
				_plan = IntPtr.Zero;
				_pinIn.Free();
				_pinOut.Free();
			}
		}
	}
}