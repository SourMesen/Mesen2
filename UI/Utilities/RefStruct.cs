using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class RefStruct<T> where T : struct
	{
		private T _value;

		public RefStruct(T value)
		{
			_value = value;
		}

		public T Get()
		{
			return _value;
		}
	}
}
