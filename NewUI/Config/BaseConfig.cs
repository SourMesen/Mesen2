using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class BaseConfig<T> : ReactiveObject
	{
		public T Clone()
		{
			return (T)this.MemberwiseClone();
		}
	}
}
