using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class JsonHelper
	{
		public static T Clone<T>(T obj)
		{
			using MemoryStream stream = new MemoryStream();
			byte[] jsonData = JsonSerializer.SerializeToUtf8Bytes(obj);
			T? clone = JsonSerializer.Deserialize<T>(jsonData);
			if(clone == null) {
				throw new Exception("Invalid object");
			}
			return clone;
		}
	}
}
