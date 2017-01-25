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
using System.Text;
using System.Numerics;

namespace FFTW.NET
{
	public static class DFT
	{
		static readonly WeakReference<Array<Complex>> _buffer = new WeakReference<Array<Complex>>(null);

		static Array<Complex> GetInitializationBuffer(long size)
		{
			Array<Complex> buffer;
			lock (_buffer)
			{
				if (!_buffer.TryGetTarget(out buffer) || buffer == null || buffer.Buffer.LongLength < size)
				{
					buffer = new Array<Complex>(new Complex[size]);
					_buffer.SetTarget(buffer);
				}
			}
			return buffer;
		}

		static void Transform(Array<Complex> input, Array<Complex> output, DftDirection direction, PlannerFlags plannerFlags, int nThreads)
		{
			if ((plannerFlags & PlannerFlags.Estimate) == PlannerFlags.Estimate)
			{
				using (var plan = FftwPlan.Create(input, output, direction, plannerFlags, nThreads))
				{
					plan.Execute();
					return;
				}
			}

			using (var plan = FftwPlan.Create(input, output, direction, plannerFlags | PlannerFlags.WisdomOnly, nThreads))
			{
				if (plan != null)
				{
					plan.Execute();
					return;
				}
			}

			/// If with <see cref="PlannerFlags.WisdomOnly"/> no plan can be created
			/// and <see cref="PlannerFlags.Estimate"/> is not specified, we use
			/// a different buffer to avoid overwriting the input
			if (input != output)
			{
				using (var plan = FftwPlan.Create(output, output, input.Rank, input.GetSize(), direction, plannerFlags, nThreads))
				{
					input.CopyTo(output);
					plan.Execute();
				}
			}
			else
			{
				var buffer = GetInitializationBuffer(input.LongLength);
				lock (buffer)
				{
					using (var plan = FftwPlan.Create(buffer, buffer, input.Rank, input.GetSize(), direction, plannerFlags, nThreads))
					{
						input.CopyTo(buffer);
						plan.Execute();
						buffer.CopyTo(output, 0, 0, input.LongLength);
					}
				}
			}
		}

		/// <summary>
		/// Performs a fast fourier transformation. The dimension is inferred from the input (<see cref="Array{T}.Rank"/>).
		/// </summary>
		public static void FFT(Array<Complex> input, Array<Complex> output, PlannerFlags plannerFlags = PlannerFlags.Default, int nThreads = 1) => Transform(input, output, DftDirection.Forwards, plannerFlags, nThreads);

		/// <summary>
		/// Performs a inverse fast fourier transformation. The dimension is inferred from the input (<see cref="Array{T}.Rank"/>).
		/// </summary>
		public static void IFFT(Array<Complex> input, Array<Complex> output, PlannerFlags plannerFlags = PlannerFlags.Default, int nThreads = 1) => Transform(input, output, DftDirection.Backwards, plannerFlags, nThreads);

		public static class Wisdom
		{
			public static bool Export(string filename) => FftwInterop.fftw_export_wisdom_to_filename(filename);
			public static bool Import(string filename) => FftwInterop.fftw_import_wisdom_from_filename(filename);
			public static void Clear() => FftwInterop.fftw_forget_wisdom();

			public static string Current
			{
				get
				{
					// We cannot use the fftw_export_wisdom_to_string function here
					// because we have no way of releasing the returned memory.
					StringBuilder sb = new StringBuilder();
					FftwInterop.WriteCharHandler writeChar = (c, ptr) => sb.Append(Convert.ToChar(c));
					FftwInterop.fftw_export_wisdom(writeChar, IntPtr.Zero);
					return sb.ToString();
				}
				set
				{
					if (string.IsNullOrEmpty(value))
						FftwInterop.fftw_forget_wisdom();
					else if (!FftwInterop.fftw_import_wisdom_from_string(value))
						throw new FormatException();
				}
			}
		}
	}
}
