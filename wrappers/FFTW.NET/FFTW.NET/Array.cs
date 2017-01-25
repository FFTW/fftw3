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
using System.Runtime.InteropServices;

namespace FFTW.NET
{
	/// <summary>
	/// Wrapper around <see cref="System.Array"/> which provides some level of type-safety.
	/// If you have access to the strongly typed instance of the underlying buffer (e.g. T[], T[,], T[,,], etc.)
	/// you should always get and set values using the strongly typed instance. <see cref="Array{T}"/>
	/// uses <see cref="Array.SetValue"/>/<see cref="Array.GetValue"/> internally which is much slower than
	/// using the strongly typed instance directly.
	/// </summary>
	public class Array<T> where T : struct
	{
		readonly System.Array _buffer;

		public int Rank => _buffer.Rank;
		public Array Buffer => _buffer;
		public int Length => _buffer.Length;
		public long LongLength => _buffer.LongLength;

		public Array(params int[] lengths)
		{
			_buffer = Array.CreateInstance(typeof(T), lengths);
		}

		public Array(Array array)
		{
			if (array.GetType().GetElementType() != typeof(T))
				throw new ArgumentException($"Must have elements of type {typeof(T).FullName}.", nameof(array));
			_buffer = array;
		}

		public int GetLength(int dimension) => _buffer.GetLength(dimension);

		public int[] GetSize()
		{
			int[] result = new int[Rank];
			for (int i = 0; i < Rank; i++)
				result[i] = GetLength(i);
			return result;
		}

		public T this[params int[] indices]
		{
			get { return (T)_buffer.GetValue(indices); }
			set { _buffer.SetValue(value, indices); }
		}

		public T this[int index]
		{
			get { return (T)_buffer.GetValue(index); }
			set { _buffer.SetValue(value, index); }
		}

		public T this[int i1, int i2]
		{
			get { return (T)_buffer.GetValue(i1, i2); }
			set { _buffer.SetValue(value, i1, i2); }
		}

		public T this[int i1, int i2, int i3]
		{
			get { return (T)_buffer.GetValue(i1, i2, i3); }
			set { _buffer.SetValue(value, i1, i2, i3); }
		}

		public static implicit operator Array(Array<T> array) => array._buffer;
		public static explicit operator Array<T>(Array array) => new Array<T>(array);

		public void CopyTo(Array<T> dest, long srcIndex, long dstIndex, long count)
		{
			if (count < 0 || count > _buffer.LongLength)
				throw new ArgumentOutOfRangeException(nameof(count));
			if (count > dest._buffer.LongLength)
				throw new ArgumentException(nameof(dest), "Destination is not large enough.");
			if (srcIndex + count > _buffer.LongLength)
				throw new ArgumentOutOfRangeException(nameof(srcIndex));
			if (dstIndex + count > dest._buffer.LongLength)
				throw new ArgumentOutOfRangeException(nameof(dstIndex));

			int sizeOfT = Marshal.SizeOf<T>();

			using (var pinSrc = PinnedGCHandle.Pin(this._buffer))
			using (var pinDst = PinnedGCHandle.Pin(dest._buffer))
			{
				unsafe
				{
					void* pSrc = new IntPtr(pinSrc.Pointer.ToInt64() + srcIndex).ToPointer();
					void* pDst = new IntPtr(pinDst.Pointer.ToInt64() + dstIndex).ToPointer();
					System.Buffer.MemoryCopy(pSrc, pDst, dest._buffer.LongLength * sizeOfT, count * sizeOfT);
				}
			}
		}

		public void CopyTo(Array<T> dest, int[] srcIndices, int[] dstIndices, long count)
		{
			if (srcIndices == null)
				throw new ArgumentNullException(nameof(srcIndices));
			if (dstIndices == null)
				throw new ArgumentNullException(nameof(dstIndices));

			long srcIndex = GetIndex(srcIndices);
			long dstIndex = dest.GetIndex(dstIndices);
			this.CopyTo(dest, srcIndex, dstIndex, count);
		}

		public void CopyTo(Array<T> dest) => this.CopyTo(dest, 0, 0, _buffer.LongLength);

		public long GetIndex(int[] indices)
		{
			if (indices.Length != Rank)
				throw new ArgumentException($"Array of length {nameof(Rank)} = {Rank} expected.", nameof(indices));
			long index = indices[0];
			for (int i = 1; i < indices.Length; i++)
			{
				index *= GetLength(i);
				index += indices[i];
			}
			return index;
		}

		public override bool Equals(object obj) => (obj as Array<T>) == this;

		public override int GetHashCode() => _buffer.GetHashCode();

		public static bool operator ==(Array<T> a, Array<T> b) => a?._buffer == b?._buffer;
		public static bool operator !=(Array<T> a, Array<T> b) => a?._buffer != b?._buffer;
	}
}