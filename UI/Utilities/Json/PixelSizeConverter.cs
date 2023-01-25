using Avalonia;
using System;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Mesen.Utilities.Json
{
	public class PixelSizeConverter : JsonConverter<PixelSize>
	{
		public override PixelSize Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			int width = 0;
			int height = 0;

			int depth = reader.CurrentDepth;
			while(reader.Read() && reader.CurrentDepth > depth) {
				if(reader.TokenType == JsonTokenType.PropertyName) {
					if(reader.GetString() == nameof(PixelSize.Width)) {
						reader.Read();
						width = reader.GetInt32();
					} else if(reader.GetString() == nameof(PixelSize.Height)) {
						reader.Read();
						height = reader.GetInt32();
					}
				}
			}

			return new PixelSize(width, height);
		}

		public override void Write(Utf8JsonWriter writer, PixelSize value, JsonSerializerOptions options)
		{
			writer.WriteStartObject();
			writer.WriteNumber(nameof(PixelSize.Width), value.Width);
			writer.WriteNumber(nameof(PixelSize.Height), value.Height);
			writer.WriteEndObject();
		}
	}
}
