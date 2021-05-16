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
			if(this is T obj) {
				return JsonHelper.Clone<T>(obj);
			} else {
				throw new InvalidCastException();
			}
		}
	}
}
