using Mesen.Utilities.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class JsonHelper
	{
		public static JsonSerializerOptions Options = new JsonSerializerOptions() {
			Converters = { new TimeSpanConverter(), new JsonStringEnumConverter(), new ColorConverter(), new PixelSizeConverter(), new PixelPointConverter() },
			WriteIndented = true,
			IgnoreReadOnlyProperties = true,
			IncludeFields = true 
		};

		public static T Clone<T>(T obj) where T : notnull
		{
			using MemoryStream stream = new MemoryStream();
			byte[] jsonData = JsonSerializer.SerializeToUtf8Bytes(obj, JsonHelper.Options);
			T? clone = (T?)JsonSerializer.Deserialize(jsonData.AsSpan<byte>(), obj.GetType(), JsonHelper.Options);
			if(clone == null) {
				throw new Exception("Invalid object");
			}
			return clone;
		}
	}
}
