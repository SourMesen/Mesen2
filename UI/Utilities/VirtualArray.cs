using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class VirtualArray<T> where T : struct
	{
		private const int BatchSize = 0x10000;

		private int _size;
		private int _loadedPage = -1;
		private T[] _pageData;
		private Func<int, int, T[]> _loader;

		public VirtualArray(int size, Func<int, int, T[]> loader)
		{
			_size = size;
			_pageData = new T[0];			
			_loader = loader;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining | MethodImplOptions.AggressiveOptimization)]
		public T Get(int i)
		{
			if(_loadedPage != (i & ~(BatchSize - 1))) {
				LoadPage(i);
			}
			return _pageData[i - _loadedPage];
		}

		[MethodImpl(MethodImplOptions.NoInlining)]
		private void LoadPage(int i)
		{
			int start = i / BatchSize * BatchSize;
			int end = Math.Min(_size, start + BatchSize) - 1;
			_pageData = _loader(start, end);
			_loadedPage = start;
		}

		public T this[int i]
		{
			get => Get(i);
		}
	}
}
