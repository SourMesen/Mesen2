using Avalonia;
using System;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Mesen.Utilities.Json
{
	public class PixelPointConverter : JsonConverter<PixelPoint>
	{
		public override PixelPoint Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			int x = 0;
			int y = 0;

			int depth = reader.CurrentDepth;
			while(reader.Read() && reader.CurrentDepth > depth) {
				if(reader.TokenType == JsonTokenType.PropertyName) {
					if(reader.GetString() == nameof(PixelPoint.X)) {
						reader.Read();
						x = reader.GetInt32();
					} else if(reader.GetString() == nameof(PixelPoint.Y)) {
						reader.Read();
						y = reader.GetInt32();
					}
				}
			}

			return new PixelPoint(x, y);
		}

		public override void Write(Utf8JsonWriter writer, PixelPoint value, JsonSerializerOptions options)
		{
			writer.WriteStartObject();
			writer.WriteNumber(nameof(PixelPoint.X), value.X);
			writer.WriteNumber(nameof(PixelPoint.Y), value.Y);
			writer.WriteEndObject();
		}
	}
}
