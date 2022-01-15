using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Mesen.Config.Shortcuts;
using Avalonia.LogicalTree;
using Mesen.Interop;
using Avalonia.Styling;
using System.IO;
using Avalonia.Data.Converters;
using Avalonia.Input;

namespace Mesen.Controls
{
	public class MesenNumericTextBox : TextBox, IStyleable
	{
		Type IStyleable.StyleKey => typeof(TextBox);

		public static readonly StyledProperty<IValueConverter?> ConverterProperty = AvaloniaProperty.Register<MesenNumericTextBox, IValueConverter?>(nameof(Converter));
		public static readonly StyledProperty<IComparable> ValueProperty = AvaloniaProperty.Register<MesenNumericTextBox, IComparable>(nameof(Value), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<IComparable?> MinProperty = AvaloniaProperty.Register<MesenNumericTextBox, IComparable?>(nameof(Min), null);
		public static readonly StyledProperty<IComparable?> MaxProperty = AvaloniaProperty.Register<MesenNumericTextBox, IComparable?>(nameof(Max), null);

		public IValueConverter? Converter
		{
			get { return GetValue(ConverterProperty); }
			set { SetValue(ConverterProperty, value); }
		}

		public IComparable Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public IComparable? Min
		{
			get { return GetValue(MinProperty); }
			set { SetValue(MinProperty, value); }
		}

		public IComparable? Max
		{
			get { return GetValue(MaxProperty); }
			set { SetValue(MaxProperty, value); }
		}

		static MesenNumericTextBox()
		{
			ValueProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				x.UpdateText();
			});
		}

		public MesenNumericTextBox()
		{
		}

		protected override void OnTextInput(TextInputEventArgs e)
		{
			base.OnTextInput(e);
			UpdateValue();
			UpdateText();
		}

		private void UpdateValue()
		{
			IComparable? val;
			if(Converter != null) {
				val = (IComparable?)Converter.ConvertBack(Text, Value.GetType(), null, System.Globalization.CultureInfo.InvariantCulture);
			} else {
				if(!long.TryParse(Text, out long parsedValue)) {
					val = Value;
				} else {
					val = parsedValue;
				}
			}

			if(val != null) {
				if(Max != null && val.CompareTo(Convert.ChangeType(Max, val.GetType())) > 0) {
					val = (IComparable)Convert.ChangeType(Max, val.GetType());
				} else if(Min != null && val.CompareTo(Convert.ChangeType(Min, val.GetType())) < 0) {
					val = (IComparable)Convert.ChangeType(Min, val.GetType());
				} else if(Min == null && val.CompareTo(Convert.ChangeType(0, val.GetType())) < 0) {
					val = 0;
				}

				Value = val;
			}
		}

		private void UpdateText(bool force = false)
		{
			string? text;
			if(Converter != null) {
				text = (string?)Converter.Convert(Value, typeof(string), null, System.Globalization.CultureInfo.InvariantCulture);
			} else {
				text = Value.ToString();
			}

			if(force || text?.Trim('0', ' ') != Text?.Trim('0', ' ')) {
				Text = text;
			}
		}

		protected override void OnLostFocus(RoutedEventArgs e)
		{
			base.OnLostFocus(e);
			UpdateValue();
			UpdateText(true);
		}
	}
}
