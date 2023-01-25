using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class EnumExtensions
	{
		public static T? GetAttribute<T>(this Enum val) where T : Attribute
		{
			return val.GetType().GetMember(val.ToString())[0].GetCustomAttribute<T>();
		}
	}
}
