using System;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Mesen.Config
{
	public class TimeSpanConverter : JsonConverter<TimeSpan>
	{
		public override TimeSpan Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			string? value = reader.GetString();
			if(value == null) {
				throw new ArgumentException("Invalid value for timespan");
			}
			return TimeSpan.Parse(value);
		}

		public override void Write(Utf8JsonWriter writer, TimeSpan value, JsonSerializerOptions options)
		{
			writer.WriteStringValue(value.ToString());
		}
	}
}
