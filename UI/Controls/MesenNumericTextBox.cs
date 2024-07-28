using Avalonia;
using Avalonia.Controls;
using System;
using Avalonia.Interactivity;
using Avalonia.Styling;
using Avalonia.Input;
using Mesen.Debugger.Utilities;
using System.Globalization;
using Avalonia.Threading;

namespace Mesen.Controls
{
	public class MesenNumericTextBox : TextBox
	{
		protected override Type StyleKeyOverride => typeof(TextBox);

		private static HexConverter _hexConverter = new HexConverter();
		
		public static readonly StyledProperty<bool> TrimProperty = AvaloniaProperty.Register<MesenNumericTextBox, bool>(nameof(Trim));
		public static readonly StyledProperty<bool> HexProperty = AvaloniaProperty.Register<MesenNumericTextBox, bool>(nameof(Hex));
		public static readonly StyledProperty<IComparable> ValueProperty = AvaloniaProperty.Register<MesenNumericTextBox, IComparable>(nameof(Value), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<string?> MinProperty = AvaloniaProperty.Register<MesenNumericTextBox, string?>(nameof(Min), null);
		public static readonly StyledProperty<string?> MaxProperty = AvaloniaProperty.Register<MesenNumericTextBox, string?>(nameof(Max), null);

		private bool _preventTextUpdate;

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

		public string? Min
		{
			get { return GetValue(MinProperty); }
			set { SetValue(MinProperty, value); }
		}

		public string? Max
		{
			get { return GetValue(MaxProperty); }
			set { SetValue(MaxProperty, value); }
		}

		static MesenNumericTextBox()
		{
			ValueProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				if(!x.IsInitialized) {
					return;
				}

				//This seems to sometimes cause a stack overflow when the code tries to update
				//value based on the min/max values, which seems to trigger an infinite loop
				//of value updates (unsure if this is an Avalonia bug?) - updating after the event
				//prevents the stack overflow/crash.
				Dispatcher.UIThread.Post(() => {
					x.SetNewValue(x.Value);
					x.UpdateText();
					x.MaxLength = x.GetMaxLength();
				});
			});

			MaxProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				x.MaxLength = x.GetMaxLength();
				x.UpdateText(true);
			});


			TextProperty.Changed.AddClassHandler<MesenNumericTextBox>((x, e) => {
				if(!x.IsInitialized) {
					return;
				}

				//Only update internal value while user is actively editing the text
				//Text will be update to its "standard" representation when focus is lost
				x._preventTextUpdate = true;
				x.UpdateValueFromText();
				x._preventTextUpdate = false;
			});
		}

		public MesenNumericTextBox()
		{
		}
		
		protected override void OnInitialized()
		{
			base.OnInitialized();
			MaxLength = GetMaxLength();
			UpdateText(true);
		}

		long? GetMin()
		{
			return GetConvertedMinMaxValue(Min);
		}

		long? GetMax()
		{
			return GetConvertedMinMaxValue(Max);
		}

		private long? GetConvertedMinMaxValue(string? valStr)
		{
			if(valStr != null) {
				NumberStyles styles = NumberStyles.Integer;
				if(valStr.StartsWith("0x")) {
					valStr = valStr.Substring(2);
					styles = NumberStyles.HexNumber;
				}
				if(long.TryParse(valStr, styles, null, out long val)) {
					return val;
				}
			}
			return null;
		}

		protected override void OnTextInput(TextInputEventArgs e)
		{
			if(e.Text == null) {
				e.Handled = true;
				return;
			}

			long? min = GetMin();
			bool allowNegative = min != null && min.Value < 0;

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
						if(c == '-' && allowNegative) {
							//Allow negative sign
							continue;
						}

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
				long? min = GetMin();
				if(min != null && min.Value > 0) {
					SetNewValue((IComparable)Convert.ChangeType(min.Value, Value.GetType()));
				} else {
					SetNewValue((IComparable)Convert.ChangeType(0, Value.GetType()));
				}
			}

			IComparable? val;
			if(Hex) {
				val = (IComparable?)_hexConverter.ConvertBack(Text, Value.GetType(), null, CultureInfo.InvariantCulture);
			} else {
				if(!long.TryParse(Text, out long parsedValue)) {
					val = Value;
				} else {
					if(parsedValue == 0 && Text.StartsWith("-")) {
						//Allow typing minus before a 0, turn value into -1
						val = -1;
					} else {
						val = parsedValue;
					}
				}
			}

			if(val != null) {
				SetNewValue(val);
			}
		}

		private int GetMaxLength()
		{
			IFormattable max;
			long? maxProp = GetMax();
			if(maxProp != null) {
				max = maxProp.Value;
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
					_ => int.MaxValue
				};
			}

			//Increase max length by 1 if minus signs are allowed
			long? min = GetMin();
			bool allowNegative = !Hex && min != null && min.Value < 0;
			return max.ToString(Hex ? "X" : null, null).Length + (allowNegative ? 1 : 0);
		}

		private void SetNewValue(IComparable val)
		{
			if(val == null) {
				return;
			}

			long? max = GetMax();
			long? min = GetMin();
			
			if(max != null && val.CompareTo(Convert.ChangeType(max, val.GetType())) > 0) {
				val = (IComparable)Convert.ChangeType(max, val.GetType());
			} else if(min != null && val.CompareTo(Convert.ChangeType(min, val.GetType())) < 0) {
				val = (IComparable)Convert.ChangeType(min, val.GetType());
			} else if(min == null && val.CompareTo(Convert.ChangeType(0, val.GetType())) < 0) {
				val = (IComparable)Convert.ChangeType(0, val.GetType());
			}
			if(!object.Equals(Value, val)) {
				Value = val;
			}
		}

		private void UpdateText(bool force = false)
		{
			if(Value == null || _preventTextUpdate) {
				return;
			}

			string? text;
			if(Hex) {
				string format = "X" + MaxLength;
				text = (string?)_hexConverter.Convert(Value, typeof(string), format, CultureInfo.InvariantCulture);
			} else {
				text = Value.ToString();
			}

			if(force || text?.TrimStart('0', ' ').ToLowerInvariant() != Text?.TrimStart('0', ' ').ToLowerInvariant()) {
				if(Trim) {
					text = text?.TrimStart('0', ' ');
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
