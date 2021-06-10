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
		private static JsonSerializerOptions _options = new JsonSerializerOptions() { IgnoreReadOnlyProperties = true, IncludeFields = true };

		public static T Clone<T>(T obj)
		{
			using MemoryStream stream = new MemoryStream();
			byte[] jsonData = JsonSerializer.SerializeToUtf8Bytes(obj, _options);
			T? clone = JsonSerializer.Deserialize<T>(jsonData, _options);
			if(clone == null) {
				throw new Exception("Invalid object");
			}
			return clone;
		}
	}
}
