using Avalonia;
using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace Mesen.Debugger.Utilities
{
	public class HexConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(targetType == typeof(string)) {
				if(parameter is string format && value is IFormattable f) {
					if(format.StartsWith("$")) {
						return "$" + f.ToString(format.Substring(1), null);
					} else {
						return f.ToString(format, null);
					}
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


		public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
		{
			if(value is string s) {
				if(s.StartsWith("$")) {
					s = s.Substring(1);
				}

				long.TryParse(s, NumberStyles.HexNumber, null, out long v);

				if(targetType == typeof(byte)) {
					return (byte)Math.Min(byte.MaxValue, Math.Max(v, byte.MinValue));
				} else if(targetType == typeof(sbyte)) {
					return (sbyte)Math.Min(sbyte.MaxValue, Math.Max(v, sbyte.MinValue));
				} else if(targetType == typeof(Int16)) {
					return (Int16)Math.Min(Int16.MaxValue, Math.Max(v, Int16.MinValue));
				} else if(targetType == typeof(UInt16)) {
					return (UInt16)Math.Min(UInt16.MaxValue, Math.Max(v, UInt16.MinValue));
				} else if(targetType == typeof(Int32)) {
					return (Int32)Math.Min(Int32.MaxValue, Math.Max(v, Int32.MinValue));
				} else if(targetType == typeof(UInt32)) {
					return (UInt32)Math.Min(UInt32.MaxValue, Math.Max(v, UInt32.MinValue));
				}
			}

			return 0;
		}
	}
}
