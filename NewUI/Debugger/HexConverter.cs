using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace Mesen.Debugger
{
	public class HexConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if(targetType == typeof(string)) {
				if(parameter is string format && value is IFormattable f) {
					return f.ToString(format, null);
				}

				if(value is byte || value is sbyte) {
					return ((IFormattable)value).ToString("X2", null);
				} else if(value is Int16 || value is UInt16) {
					return ((IFormattable)value).ToString("X4", null);
				} else if(value is Int32 || value is UInt32) {
					return ((IFormattable)value).ToString("X8", null);
				}
			}

			throw new Exception("unsupported");
		}


		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if(value is string s) {
				if(targetType == typeof(byte)) {
					byte.TryParse(s, NumberStyles.HexNumber, null, out byte v);
					return v;
				} else if(targetType == typeof(sbyte)) {
					sbyte.TryParse(s, NumberStyles.HexNumber, null, out sbyte v);
					return v;
				} else if(targetType == typeof(Int16)) {
					Int16.TryParse(s, NumberStyles.HexNumber, null, out Int16 v);
					return v;
				} else if(targetType == typeof(UInt16)) {
					UInt16.TryParse(s, NumberStyles.HexNumber, null, out UInt16 v);
					return v;
				} else if(targetType == typeof(Int32)) {
					Int32.TryParse(s, NumberStyles.HexNumber, null, out Int32 v);
					return v;
				} else if(targetType == typeof(UInt32)) {
					UInt32.TryParse(s, NumberStyles.HexNumber, null, out UInt32 v);
					return v;
				}
			}

			return 0;
		}
	}
}
