using Avalonia;
using Avalonia.Data.Converters;
using Avalonia.Media;
using System;
using System.Diagnostics;
using System.Globalization;

namespace Mesen.Utilities
{
	public class EnumMatchConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is Enum val && parameter is Enum compare && targetType == typeof(bool)) {
				return object.Equals(val, compare);
			}
			throw new Exception("invalid conversion");
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			throw new Exception("invalid conversion");
		}
	}
}
