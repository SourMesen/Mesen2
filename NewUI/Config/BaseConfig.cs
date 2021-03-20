using Mesen.Utilities;
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
	public class BaseConfig<T> : ReactiveObject where T : class
	{
		public T Clone() 
		{
			return JsonHelper.Clone<T>(this as T);
		}
	}
}
