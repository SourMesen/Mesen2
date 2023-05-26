using Avalonia.Media;
using System;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Mesen.Utilities.Json
{
	public class ColorConverter : JsonConverter<Color>
	{
		public override Color Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			UInt32 value = reader.GetUInt32();
			return Color.FromUInt32(value);
		}

		public override void Write(Utf8JsonWriter writer, Color value, JsonSerializerOptions options)
		{
			writer.WriteNumberValue(value.ToUInt32());
		}
	}
}
