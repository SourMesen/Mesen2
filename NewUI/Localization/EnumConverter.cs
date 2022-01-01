using Avalonia.Data.Converters;
using System;

namespace Mesen.Localization
{
	public class EnumConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(value is Enum && targetType == typeof(string)) {
				return ResourceHelper.GetEnumText((Enum)value);
			}

			return value?.ToString() ?? "null value";
		}


		public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			throw new NotImplementedException();
		}
	}
}	
