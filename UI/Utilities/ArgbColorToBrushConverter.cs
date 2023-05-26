using Avalonia;
using Avalonia.Data.Converters;
using Avalonia.Media;
using System;
using System.Globalization;

namespace Mesen.Utilities
{
	public class ArgbColorToBrushConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is UInt32 c && targetType == typeof(IBrush)) {
				return new SolidColorBrush(c);
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is SolidColorBrush b && targetType == typeof(UInt32)) {
				return b.Color.ToUInt32();
			}
			return AvaloniaProperty.UnsetValue;
		}
	}
}
