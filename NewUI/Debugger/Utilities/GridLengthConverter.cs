using Avalonia.Controls;
using Avalonia.Data.Converters;
using System;

namespace Mesen.Debugger.Utilities
{
	public class GridLengthConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(value is double val && targetType == typeof(GridLength)) {
				return new GridLength(val, GridUnitType.Pixel);
			}

			throw new Exception("unsupported");
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(value is GridLength s) {
				return s.Value;
			}

			return 0.0;
		}
	}
}
