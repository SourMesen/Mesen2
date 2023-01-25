using Avalonia;
using Avalonia.Data.Converters;
using Avalonia.Media;
using System;
using System.Globalization;

namespace Mesen.Utilities
{
	public class FontNameConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is string fontName && targetType == typeof(FontFamily)) {
				return new FontFamily(fontName);
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is FontFamily font && targetType == typeof(string)) {
				return font.Name;
			}
			return AvaloniaProperty.UnsetValue;
		}
	}
}
