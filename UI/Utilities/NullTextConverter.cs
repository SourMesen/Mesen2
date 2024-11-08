using Avalonia.Controls;
using Avalonia.Data.Converters;
using System;

namespace Mesen.Utilities;

/// <summary>
/// Used to fix an issue with undo in Avalonia's TextBox.
/// Typing a string, then undoing multiple times will eventually set
/// the string to null instead of an empty string, causing crashes
/// in code that does not expect the string to ever be null.
/// </summary>
public class NullTextConverter : IValueConverter
{
	public object Convert(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
	{
		if(value is string val) {
			return val ?? "";
		}
		return "";
	}

	public object ConvertBack(object? value, Type targetType, object? parameter, System.Globalization.CultureInfo culture)
	{
		if(value is string s) {
			return s;
		}
		return "";
	}
}
