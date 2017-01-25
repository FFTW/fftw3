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
	public class FftwPlan : IDisposable
	{
		IntPtr _plan = IntPtr.Zero;

		readonly Array<Complex> _input;
		readonly Array<Complex> _output;
		readonly PinnedGCHandle _pinIn;
		readonly PinnedGCHandle _pinOut;
		
		public Array<Complex> Input => _input;
		public Array<Complex> Output => _output;

		FftwPlan(IntPtr plan, Array<Complex> input, Array<Complex> output, PinnedGCHandle pinIn, PinnedGCHandle pinOut)
		{
			_plan = plan;
			_input = input;
			_output = output;
			_pinIn = pinIn;
			_pinOut = pinOut;
		}

		public static FftwPlan Create(Array<Complex> input, Array<Complex> output, DftDirection direction, PlannerFlags plannerFlags = PlannerFlags.Default, int nThreads = 1)
		{
			if (input.Rank != output.Rank)
				throw new ArgumentException($"{nameof(input)} and {nameof(output)} must have the same rank and size.");
			for (int i = 0; i < input.Rank; i++)
			{
				if (input.GetLength(i) != output.GetLength(i))
					throw new ArgumentException($"{nameof(input)} and {nameof(output)} must have the same rank and size.");
			}

			return Create(input, output, input.Rank, input.GetSize(), direction, plannerFlags, nThreads);
		}

		/// <summary>
		/// Initializes a new plan using the provided input and output buffers.
		/// These buffers may be overwritten during initialization.
		/// </summary>
		internal static FftwPlan Create(Array<Complex> input, Array<Complex> output, int rank, int[] n, DftDirection direction, PlannerFlags plannerFlags, int nThreads)
		{
			if (!FftwInterop.IsAvailable)
				throw new InvalidOperationException($"{nameof(FftwInterop.IsAvailable)} returns false.");

			if (nThreads < 1)
				nThreads = Environment.ProcessorCount;

			var pIn = PinnedGCHandle.Pin(input.Buffer);
			var pOut = PinnedGCHandle.Pin(output.Buffer);
			IntPtr plan = IntPtr.Zero;

			lock (FftwInterop.Lock)
			{
				FftwInterop.fftw_plan_with_nthreads(nThreads);
				plan = FftwInterop.fftw_plan_dft(rank, n, pIn.Pointer, pOut.Pointer, direction, plannerFlags);
			}

			if (plan == IntPtr.Zero)
			{
				pIn.Free();
				pOut.Free();
				return null;
			}

			return new FftwPlan(plan, input, output, pIn, pOut);
		}

		public void Execute()
		{
			if (_plan == IntPtr.Zero)
				throw new ObjectDisposedException(nameof(FftwPlan));

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
