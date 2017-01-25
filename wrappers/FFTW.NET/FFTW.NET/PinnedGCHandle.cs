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
	public struct PinnedGCHandle : IDisposable
	{
		GCHandle _handle;

		PinnedGCHandle(GCHandle handle)
		{
			_handle = handle;
		}

		public static PinnedGCHandle Pin(object obj) => new PinnedGCHandle(GCHandle.Alloc(obj, GCHandleType.Pinned));

		public void Free() => _handle.Free();

		void IDisposable.Dispose()
		{
			if (_handle.IsAllocated)
				_handle.Free();
		}

		public override string ToString() => _handle.ToString();

		public IntPtr Pointer => _handle.AddrOfPinnedObject();
		public bool IsAllocated => _handle.IsAllocated;
		public object Target => _handle.Target;
	}
}
