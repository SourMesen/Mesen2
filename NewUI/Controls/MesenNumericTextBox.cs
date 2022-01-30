using Avalonia;
using Avalonia.Controls;
using System;
using Avalonia.Interactivity;
using Avalonia.Styling;
using Avalonia.Input;
using Mesen.Debugger.Utilities;

namespace Mesen.Controls
{
	public class MesenNumericTextBox : TextBox, IStyleable
	{
		Type IStyleable.StyleKey => typeof(TextBox);

		private static HexConverter _hexConverter = new HexConverter();

		public static readonly StyledProperty<bool> TrimProperty = AvaloniaProperty.Register<MesenNumericTextBox, bool>(nameof(Trim));
		public static readonly StyledProperty<bool> HexProperty = AvaloniaProperty.Register<MesenNumericTextBox, bool>(nameof(Hex));
		public static readonly StyledProperty<IComparable> ValueProperty = AvaloniaProperty.Register<MesenNumericTextBox, IComparable>(nameof(Value), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int?> MinProperty = AvaloniaProperty.Register<MesenNumericTextBox, int?>(nameof(Min), null);
		public static readonly StyledProperty<int?> MaxProperty = AvaloniaProperty.Register<MesenNumericTextBox, int?>(nameof(Max), null);

		public bool Hex
		{
			get { return GetValue(HexProperty); }
			set { SetValue(HexProperty, value); }
		}

		public bool Trim
		{
			get { return GetValue(TrimProperty); }
			set { SetValue(TrimProperty, value); }
		}

		public IComparable Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public int? Min
		{
			get { return GetValue(MinProperty); }
			set { SetValue(MinProperty, value); }
		}

		public int? Max
		{
			get { return GetValue(MaxProperty); }
			set { SetValue(MaxProperty, value); }
		}

		static MesenNumericTextBox()
		{
			ValueProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				x.UpdateText();
				x.MaxLength = x.GetMaxLength();
			});

			MaxProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				x.MaxLength = x.GetMaxLength();
				x.UpdateText(true);
			});

			TextProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				x.UpdateValueFromText();
				x.UpdateText();
			});
		}

		public MesenNumericTextBox()
		{
		}

		protected override void OnTextInput(TextInputEventArgs e)
		{
			if(e.Text == null) {
				e.Handled = true;
				return;
			}

			if(Hex) {
				foreach(char c in e.Text.ToLowerInvariant()) {
					if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
						//not hex
						e.Handled = true;
						return;
					}
				}
			} else {
				foreach(char c in e.Text) {
					if(c < '0' || c > '9') {
						//not a number
						e.Handled = true;
						return;
					}
				}
			}

			base.OnTextInput(e);
		}

		private void UpdateValueFromText()
		{
			if(string.IsNullOrWhiteSpace(Text)) {
				if(Min != null) {
					SetNewValue(Math.Min(0, Min.Value));
				} else {
					SetNewValue(0);
				}
			}

			IComparable? val;
			if(Hex) {
				val = (IComparable?)_hexConverter.ConvertBack(Text, Value.GetType(), null, System.Globalization.CultureInfo.InvariantCulture);
			} else {
				if(!long.TryParse(Text, out long parsedValue)) {
					val = Value;
				} else {
					val = parsedValue;
				}
			}

			if(val != null) {
				SetNewValue(val);
			}
		}

		private int GetMaxLength()
		{
			IFormattable max;
			if(Max != null) {
				max = Max;
			} else {
				max = Value switch {
					byte _ => byte.MaxValue,
					sbyte _ => sbyte.MaxValue,
					short _ => short.MaxValue,
					ushort _ => ushort.MaxValue,
					int _ => int.MaxValue,
					uint _ => uint.MaxValue,
					long _ => long.MaxValue,
					ulong _ => ulong.MaxValue,
					_ => throw new Exception("invalid value type")
				};
			}

			return max.ToString(Hex ? "X" : null, null).Length;
		}

		private void SetNewValue(IComparable val)
		{
			if(Max != null && val.CompareTo(Convert.ChangeType(Max, val.GetType())) > 0) {
				val = (IComparable)Convert.ChangeType(Max, val.GetType());
			} else if(Min != null && val.CompareTo(Convert.ChangeType(Min, val.GetType())) < 0) {
				val = (IComparable)Convert.ChangeType(Min, val.GetType());
			} else if(Min == null && val.CompareTo(Convert.ChangeType(0, val.GetType())) < 0) {
				val = 0;
			}

			Value = val;
		}

		private void UpdateText(bool force = false)
		{
			if(Value == null) {
				return;
			}

			string? text;
			if(Hex) {
				string format = "X" + MaxLength;
				text = (string?)_hexConverter.Convert(Value, typeof(string), format, System.Globalization.CultureInfo.InvariantCulture);
			} else {
				text = Value.ToString();
			}

			if(force || text?.Trim('0', ' ').ToLowerInvariant() != Text?.Trim('0', ' ').ToLowerInvariant()) {
				if(Trim) {
					text = text?.Trim('0', ' ');
				}

				if(text?.Length == 0) {
					text = "0";
				}
				
				Text = text;
			}
		}

		protected override void OnGotFocus(GotFocusEventArgs e)
		{
			base.OnGotFocus(e);
			this.SelectAll();
		}

		protected override void OnLostFocus(RoutedEventArgs e)
		{
			base.OnLostFocus(e);
			UpdateValueFromText();
			UpdateText(true);
		}
	}
}
