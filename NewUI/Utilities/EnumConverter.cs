using Avalonia.Data.Converters;
using Mesen.Localization;
using System;

namespace Mesen.Utilities
{
	public class EnumConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(value is Enum enumValue && targetType == typeof(string)) {
				return ResourceHelper.GetEnumText(enumValue);
			}

			throw new Exception("unsupported");
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			throw new Exception("unsupported");
		}
	}
}
