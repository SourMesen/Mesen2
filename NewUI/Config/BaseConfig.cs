using Mesen.Utilities;
using ReactiveUI;
using System;

namespace Mesen.Config
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
