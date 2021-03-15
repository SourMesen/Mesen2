using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class BaseConfig<T>
	{
		public T Clone()
		{
			return (T)this.MemberwiseClone();
		}
	}
}
