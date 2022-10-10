using Mesen.Utilities;
using ReactiveUI;
using System;
using System.Text.Json;

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

		public bool IsIdentical(T other)
		{
			string a = JsonSerializer.Serialize(this, this.GetType(), JsonHelper.Options);
			string b = JsonSerializer.Serialize(other, this.GetType(), JsonHelper.Options);
			return a == b;
		}
	}
}
