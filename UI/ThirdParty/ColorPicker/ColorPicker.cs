//From: https://github.com/wieslawsoltes/ThemeEditor/tree/master/src/ThemeEditor.Controls.ColorPicker (MIT license)
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text.RegularExpressions;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Data.Converters;
using Avalonia.Input;
using Avalonia.Media;
using ThemeEditor.Controls.ColorPicker.Colors;

namespace ThemeEditor.Controls.ColorPicker
{
	public static class ColorHelpers
	{
		private static readonly Regex s_hexRegex = new Regex("^#[a-fA-F0-9]{6}$");

		public static bool IsValidHexColor(string hex)
		{
			return !string.IsNullOrWhiteSpace(hex) && s_hexRegex.Match(hex).Success;
		}

		public static string ToHexColor(Color color)
		{
			return "#" + (color.ToUInt32() & 0xFFFFFF).ToString("X6");
		}

		public static Color FromHexColor(string hex)
		{
			return Color.Parse(hex);
		}

		public static void FromColor(Color color, out double h, out double s, out double v, out double a)
		{
			HSV hsv = new RGB(color.R, color.G, color.B).ToHSV();
			h = hsv.H;
			s = hsv.S;
			v = hsv.V;
			a = color.A * 100.0 / 255.0;
		}

		public static Color FromHSVA(double h, double s, double v, double a)
		{
			RGB rgb = new HSV(h, s, v).ToRGB();
			byte A = (byte)(a * 255.0 / 100.0);
			return new Color(A, (byte)rgb.R, (byte)rgb.G, (byte)rgb.B);
		}

		public static Color FromRGBA(byte r, byte g, byte b, double a)
		{
			byte A = (byte)(a * 255.0 / 100.0);
			return new Color(A, r, g, b);
		}
	}

	public class ColorToBrushConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is Color c && targetType == typeof(IBrush)) {
				return new SolidColorBrush(c);
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is SolidColorBrush b && targetType == typeof(Color)) {
				return b.Color;
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class HexToColorConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is string s && targetType == typeof(Color)) {
				try {
					if(ColorHelpers.IsValidHexColor(s)) {
						return ColorHelpers.FromHexColor(s);
					}
				} catch(Exception) {
					return AvaloniaProperty.UnsetValue;
				}
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is Color c && targetType == typeof(string)) {
				try {
					return ColorHelpers.ToHexColor(c);
				} catch(Exception) {
					return AvaloniaProperty.UnsetValue;
				}
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class HueConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return v * range / 360.0;
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return v * 360.0 / range;
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class SaturationConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return v * range / 100.0;
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return v * 100.0 / range;
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class ValueConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return range - (v * range / 100.0);
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double v && parameter is double range && targetType == typeof(double)) {
				return 100.0 - (v * 100.0 / range);
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class HsvaToColorConverter : IMultiValueConverter
	{
		public object Convert(IList<object?> values, Type targetType, object? parameter, CultureInfo culture)
		{
			double[] v = values.OfType<double>().ToArray();
			if(v.Length == values.Count) {
				return ColorHelpers.FromHSVA(v[0], v[1], v[2], v[3]);
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public class HueToColorConverter : IValueConverter
	{
		public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is double h && targetType == typeof(Color)) {
				return ColorHelpers.FromHSVA(h, 100, 100, 100);
			}
			return AvaloniaProperty.UnsetValue;
		}

		public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
		{
			if(value is string s && targetType == typeof(double)) {
				try {
					if(ColorHelpers.IsValidHexColor(s)) {
						ColorHelpers.FromColor(ColorHelpers.FromHexColor(s), out double h, out _, out _, out _);
						return h;
					}
				} catch(Exception) {
					return AvaloniaProperty.UnsetValue;
				}
			}
			return AvaloniaProperty.UnsetValue;
		}
	}

	public abstract class ColorPickerProperties : AvaloniaObject
	{
		public static readonly StyledProperty<ColorPicker> ColorPickerProperty =
			 AvaloniaProperty.Register<ColorPickerProperties, ColorPicker>(nameof(ColorPicker));

		public ColorPickerProperties()
		{
			this.GetObservable(ColorPickerProperty).Subscribe(x => OnColorPickerChange());
		}

		public ColorPicker ColorPicker
		{
			get { return GetValue(ColorPickerProperty); }
			set { SetValue(ColorPickerProperty, value); }
		}

		public abstract void UpdateColorPickerValues();

		public abstract void UpdatePropertyValues();

		public virtual void OnColorPickerChange()
		{
			if(ColorPicker != null) {
				ColorPicker.GetObservable(ColorPicker.Value1Property).Subscribe(x => UpdatePropertyValues());
				ColorPicker.GetObservable(ColorPicker.Value2Property).Subscribe(x => UpdatePropertyValues());
				ColorPicker.GetObservable(ColorPicker.Value3Property).Subscribe(x => UpdatePropertyValues());
				ColorPicker.GetObservable(ColorPicker.Value4Property).Subscribe(x => UpdatePropertyValues());
			}
		}
	}

	public class HsvProperties : ColorPickerProperties
	{
		public static readonly StyledProperty<double> HueProperty =
			 AvaloniaProperty.Register<HsvProperties, double>(nameof(Hue), 0.0, validate: ValidateHue);

		public static readonly StyledProperty<double> SaturationProperty =
			 AvaloniaProperty.Register<HsvProperties, double>(nameof(Saturation), 100.0, validate: ValidateSaturation);

		public static readonly StyledProperty<double> ValueProperty =
			 AvaloniaProperty.Register<HsvProperties, double>(nameof(Value), 100.0, validate: ValidateValue);

		private static bool ValidateHue(double hue)
		{
			if(hue < 0.0 || hue > 360.0) {
				throw new ArgumentException("Invalid Hue value.");
			}
			return true;
		}

		private static bool ValidateSaturation(double saturation)
		{
			if(saturation < 0.0 || saturation > 100.0) {
				throw new ArgumentException("Invalid Saturation value.");
			}
			return true;
		}

		private static bool ValidateValue(double value)
		{
			if(value < 0.0 || value > 100.0) {
				throw new ArgumentException("Invalid Value value.");
			}
			return true;
		}

		private bool _updating = false;

		public HsvProperties() : base()
		{
			this.GetObservable(HueProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(SaturationProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(ValueProperty).Subscribe(x => UpdateColorPickerValues());
		}

		public double Hue
		{
			get { return GetValue(HueProperty); }
			set { SetValue(HueProperty, value); }
		}

		public double Saturation
		{
			get { return GetValue(SaturationProperty); }
			set { SetValue(SaturationProperty, value); }
		}

		public double Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public override void UpdateColorPickerValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				ColorPicker.Value1 = Hue;
				ColorPicker.Value2 = Saturation;
				ColorPicker.Value3 = Value;
				_updating = false;
			}
		}

		public override void UpdatePropertyValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				Hue = ColorPicker.Value1;
				Saturation = ColorPicker.Value2;
				Value = ColorPicker.Value3;
				_updating = false;
			}
		}
	}

	public class RgbProperties : ColorPickerProperties
	{
		public static readonly StyledProperty<byte> RedProperty =
			 AvaloniaProperty.Register<RgbProperties, byte>(nameof(Red), 0xFF, validate: ValidateRed);

		public static readonly StyledProperty<byte> GreenProperty =
			 AvaloniaProperty.Register<RgbProperties, byte>(nameof(Green), 0x00, validate: ValidateGreen);

		public static readonly StyledProperty<byte> BlueProperty =
			 AvaloniaProperty.Register<RgbProperties, byte>(nameof(Blue), 0x00, validate: ValidateBlue);

		private static bool ValidateRed(byte red)
		{
			if(red < 0 || red > 255) {
				throw new ArgumentException("Invalid Red value.");
			}
			return true;
		}

		private static bool ValidateGreen(byte green)
		{
			if(green < 0 || green > 255) {
				throw new ArgumentException("Invalid Green value.");
			}
			return true;
		}

		private static bool ValidateBlue(byte blue)
		{
			if(blue < 0 || blue > 255) {
				throw new ArgumentException("Invalid Blue value.");
			}
			return true;
		}

		private bool _updating = false;

		public RgbProperties() : base()
		{
			this.GetObservable(RedProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(GreenProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(BlueProperty).Subscribe(x => UpdateColorPickerValues());
		}

		public byte Red
		{
			get { return GetValue(RedProperty); }
			set { SetValue(RedProperty, value); }
		}

		public byte Green
		{
			get { return GetValue(GreenProperty); }
			set { SetValue(GreenProperty, value); }
		}

		public byte Blue
		{
			get { return GetValue(BlueProperty); }
			set { SetValue(BlueProperty, value); }
		}

		public override void UpdateColorPickerValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				RGB rgb = new RGB(Red, Green, Blue);
				HSV hsv = rgb.ToHSV();
				ColorPicker.Value1 = hsv.H;
				ColorPicker.Value2 = hsv.S;
				ColorPicker.Value3 = hsv.V;
				_updating = false;
			}
		}

		public override void UpdatePropertyValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				HSV hsv = new HSV(ColorPicker.Value1, ColorPicker.Value2, ColorPicker.Value3);
				RGB rgb = hsv.ToRGB();
				Red = (byte)rgb.R;
				Green = (byte)rgb.G;
				Blue = (byte)rgb.B;
				_updating = false;
			}
		}
	}

	public class CmykProperties : ColorPickerProperties
	{
		public static readonly StyledProperty<double> CyanProperty =
			 AvaloniaProperty.Register<CmykProperties, double>(nameof(Cyan), 0.0, validate: ValidateCyan);

		public static readonly StyledProperty<double> MagentaProperty =
			 AvaloniaProperty.Register<CmykProperties, double>(nameof(Magenta), 100.0, validate: ValidateMagenta);

		public static readonly StyledProperty<double> YellowProperty =
			 AvaloniaProperty.Register<CmykProperties, double>(nameof(Yellow), 100.0, validate: ValidateYellow);

		public static readonly StyledProperty<double> BlackKeyProperty =
			 AvaloniaProperty.Register<CmykProperties, double>(nameof(BlackKey), 0.0, validate: ValidateBlackKey);

		private static bool ValidateCyan(double cyan)
		{
			if(cyan < 0.0 || cyan > 100.0) {
				throw new ArgumentException("Invalid Cyan value.");
			}
			return true;
		}

		private static bool ValidateMagenta(double magenta)
		{
			if(magenta < 0.0 || magenta > 100.0) {
				throw new ArgumentException("Invalid Magenta value.");
			}
			return true;
		}

		private static bool ValidateYellow(double yellow)
		{
			if(yellow < 0.0 || yellow > 100.0) {
				throw new ArgumentException("Invalid Yellow value.");
			}
			return true;
		}

		private static bool ValidateBlackKey(double blackKey)
		{
			if(blackKey < 0.0 || blackKey > 100.0) {
				throw new ArgumentException("Invalid BlackKey value.");
			}
			return true;
		}

		private bool _updating = false;

		public CmykProperties() : base()
		{
			this.GetObservable(CyanProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(MagentaProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(YellowProperty).Subscribe(x => UpdateColorPickerValues());
			this.GetObservable(BlackKeyProperty).Subscribe(x => UpdateColorPickerValues());
		}

		public double Cyan
		{
			get { return GetValue(CyanProperty); }
			set { SetValue(CyanProperty, value); }
		}

		public double Magenta
		{
			get { return GetValue(MagentaProperty); }
			set { SetValue(MagentaProperty, value); }
		}

		public double Yellow
		{
			get { return GetValue(YellowProperty); }
			set { SetValue(YellowProperty, value); }
		}

		public double BlackKey
		{
			get { return GetValue(BlackKeyProperty); }
			set { SetValue(BlackKeyProperty, value); }
		}

		public override void UpdateColorPickerValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				CMYK cmyk = new CMYK(Cyan, Magenta, Yellow, BlackKey);
				HSV hsv = cmyk.ToHSV();
				ColorPicker.Value1 = hsv.H;
				ColorPicker.Value2 = hsv.S;
				ColorPicker.Value3 = hsv.V;
				_updating = false;
			}
		}

		public override void UpdatePropertyValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				HSV hsv = new HSV(ColorPicker.Value1, ColorPicker.Value2, ColorPicker.Value3);
				CMYK cmyk = hsv.ToCMYK();
				Cyan = cmyk.C;
				Magenta = cmyk.M;
				Yellow = cmyk.Y;
				BlackKey = cmyk.K;
				_updating = false;
			}
		}
	}

	public class HexProperties : ColorPickerProperties
	{
		public static readonly StyledProperty<string> HexProperty =
			 AvaloniaProperty.Register<HexProperties, string>(nameof(Hex), "#FF0000", validate: ValidateHex);

		private static bool ValidateHex(string hex)
		{
			if(!ColorHelpers.IsValidHexColor(hex)) {
				throw new ArgumentException("Invalid Hex value.");
			}
			return true;
		}

		private bool _updating = false;

		public HexProperties() : base()
		{
			this.GetObservable(HexProperty).Subscribe(x => UpdateColorPickerValues());
		}

		public string Hex
		{
			get { return GetValue(HexProperty); }
			set { SetValue(HexProperty, value); }
		}

		public override void UpdateColorPickerValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				Color color = Color.Parse(Hex);
				ColorHelpers.FromColor(color, out double h, out double s, out double v, out double a);
				ColorPicker.Value1 = h;
				ColorPicker.Value2 = s;
				ColorPicker.Value3 = v;
				ColorPicker.Value4 = a;
				_updating = false;
			}
		}

		public override void UpdatePropertyValues()
		{
			if(_updating == false && ColorPicker != null) {
				_updating = true;
				var color = ColorHelpers.FromHSVA(ColorPicker.Value1, ColorPicker.Value2, ColorPicker.Value3, ColorPicker.Value4);
				Hex = ColorHelpers.ToHexColor(color);
				_updating = false;
			}
		}
	}

	public interface IValueConverters
	{
		IValueConverter Value1Converter { get; }
		IValueConverter Value2Converter { get; }
		IValueConverter Value3Converter { get; }
	}

	public class HsvValueConverters : IValueConverters
	{
		public IValueConverter Value1Converter { get; } = new HueConverter();

		public IValueConverter Value2Converter { get; } = new SaturationConverter();

		public IValueConverter Value3Converter { get; } = new ValueConverter();
	}

	public class ColorPicker : TemplatedControl
	{
		public static readonly StyledProperty<double> Value1Property =
			 AvaloniaProperty.Register<ColorPicker, double>(nameof(Value1));

		public static readonly StyledProperty<double> Value2Property =
			 AvaloniaProperty.Register<ColorPicker, double>(nameof(Value2));

		public static readonly StyledProperty<double> Value3Property =
			 AvaloniaProperty.Register<ColorPicker, double>(nameof(Value3));

		public static readonly StyledProperty<double> Value4Property =
			 AvaloniaProperty.Register<ColorPicker, double>(nameof(Value4));

		public static readonly StyledProperty<Color> ColorProperty =
			 AvaloniaProperty.Register<ColorPicker, Color>(nameof(Color));

		private Canvas? _colorCanvas;
		private Thumb? _colorThumb;
		private Canvas? _hueCanvas;
		private Thumb? _hueThumb;
		private bool _updating = false;
		private bool _captured = false;
		private readonly IValueConverters _converters = new HsvValueConverters();

		public ColorPicker()
		{
			this.GetObservable(Value1Property).Subscribe(x => OnValueChange());
			this.GetObservable(Value2Property).Subscribe(x => OnValueChange());
			this.GetObservable(Value3Property).Subscribe(x => OnValueChange());
			this.GetObservable(Value4Property).Subscribe(x => OnValueChange());
			this.GetObservable(ColorProperty).Subscribe(x => OnColorChange());
		}

		public double Value1
		{
			get { return GetValue(Value1Property); }
			set { SetValue(Value1Property, value); }
		}

		public double Value2
		{
			get { return GetValue(Value2Property); }
			set { SetValue(Value2Property, value); }
		}

		public double Value3
		{
			get { return GetValue(Value3Property); }
			set { SetValue(Value3Property, value); }
		}

		public double Value4
		{
			get { return GetValue(Value4Property); }
			set { SetValue(Value4Property, value); }
		}

		public SolidColorBrush Test
		{
			get { return new SolidColorBrush(Avalonia.Media.Colors.Blue); }
		}

		public Color Color
		{
			get { return GetValue(ColorProperty); }
			set { SetValue(ColorProperty, value); }
		}

		protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
		{
			if(_colorCanvas != null) {
				_colorCanvas.PointerPressed -= ColorCanvas_PointerPressed;
				_colorCanvas.PointerReleased -= ColorCanvas_PointerReleased;
				_colorCanvas.PointerMoved -= ColorCanvas_PointerMoved;
			}

			if(_colorThumb != null) {
				_colorThumb.DragDelta -= ColorThumb_DragDelta;
			}

			if(_hueCanvas != null) {
				_hueCanvas.PointerPressed -= HueCanvas_PointerPressed;
				_hueCanvas.PointerReleased -= HueCanvas_PointerReleased;
				_hueCanvas.PointerMoved -= HueCanvas_PointerMoved;
			}

			if(_hueThumb != null) {
				_hueThumb.DragDelta -= HueThumb_DragDelta;
			}

			_colorCanvas = e.NameScope.Find<Canvas>("PART_ColorCanvas");
			_colorThumb = e.NameScope.Find<Thumb>("PART_ColorThumb");
			_hueCanvas = e.NameScope.Find<Canvas>("PART_HueCanvas");
			_hueThumb = e.NameScope.Find<Thumb>("PART_HueThumb");

			if(_colorCanvas != null) {
				_colorCanvas.PointerPressed += ColorCanvas_PointerPressed;
				_colorCanvas.PointerReleased += ColorCanvas_PointerReleased;
				_colorCanvas.PointerMoved += ColorCanvas_PointerMoved;
			}

			if(_colorThumb != null) {
				_colorThumb.DragDelta += ColorThumb_DragDelta;
			}

			if(_hueCanvas != null) {
				_hueCanvas.PointerPressed += HueCanvas_PointerPressed;
				_hueCanvas.PointerReleased += HueCanvas_PointerReleased;
				_hueCanvas.PointerMoved += HueCanvas_PointerMoved;
			}

			if(_hueThumb != null) {
				_hueThumb.DragDelta += HueThumb_DragDelta;
			}
		}

		protected override Size ArrangeOverride(Size finalSize)
		{
			var size = base.ArrangeOverride(finalSize);
			OnColorChange();
			return size;
		}

		private bool IsTemplateValid()
		{
			return _colorCanvas != null
				 && _colorThumb != null
				 && _hueCanvas != null
				 && _hueThumb != null;
		}

		private double Clamp(double val, double min, double max)
		{
			return Math.Min(Math.Max(val, min), max);
		}

		private void MoveThumb(Canvas? canvas, Thumb? thumb, double x, double y)
		{
			if(canvas != null && thumb != null) {
				double left = Clamp(x, 0, canvas.Bounds.Width);
				double top = Clamp(y, 0, canvas.Bounds.Height);
				Canvas.SetLeft(thumb, left);
				Canvas.SetTop(thumb, top);
			}
		}

		private T? Convert<T>(IValueConverter converter, T value, T range)
		{
			return (T?)converter.Convert(value, typeof(T), range, CultureInfo.CurrentCulture);
		}

		private T? ConvertBack<T>(IValueConverter converter, T value, T range)
		{
			return (T?)converter.ConvertBack(value, typeof(T), range, CultureInfo.CurrentCulture);
		}

		private double GetValue1Range() => _hueCanvas?.Bounds.Height ?? 0.0;

		private double GetValue2Range() => _colorCanvas?.Bounds.Width ?? 0.0;

		private double GetValue3Range() => _colorCanvas?.Bounds.Height ?? 0.0;

		private void UpdateThumbsFromColor()
		{
			ColorHelpers.FromColor(Color, out double h, out double s, out double v, out double a);
			double hueY = Convert(_converters.Value1Converter, h, GetValue1Range());
			double colorX = Convert(_converters.Value2Converter, s, GetValue2Range());
			double colorY = Convert(_converters.Value3Converter, v, GetValue3Range());
			MoveThumb(_hueCanvas, _hueThumb, 0, hueY);
			MoveThumb(_colorCanvas, _colorThumb, colorX, colorY);
		}

		private void UpdateThumbsFromValues()
		{
			double hueY = Convert(_converters.Value1Converter, Value1, GetValue1Range());
			double colorX = Convert(_converters.Value2Converter, Value2, GetValue2Range());
			double colorY = Convert(_converters.Value3Converter, Value3, GetValue3Range());
			MoveThumb(_hueCanvas, _hueThumb, 0, hueY);
			MoveThumb(_colorCanvas, _colorThumb, colorX, colorY);
		}

		private void UpdateValuesFromThumbs()
		{
			if(_hueThumb == null || _colorThumb == null) {
				return;
			}

			double hueY = Canvas.GetTop(_hueThumb);
			double colorX = Canvas.GetLeft(_colorThumb);
			double colorY = Canvas.GetTop(_colorThumb);
			Value1 = ConvertBack(_converters.Value1Converter, hueY, GetValue1Range());
			Value2 = ConvertBack(_converters.Value2Converter, colorX, GetValue2Range());
			Value3 = ConvertBack(_converters.Value3Converter, colorY, GetValue3Range());
			Value4 = 100;
			Color = ColorHelpers.FromHSVA(Value1, Value2, Value3, Value4);
		}

		private void UpdateColorFromThumbs()
		{
			if(_hueThumb == null || _colorThumb == null) {
				return;
			}

			double hueY = Canvas.GetTop(_hueThumb);
			double colorX = Canvas.GetLeft(_colorThumb);
			double colorY = Canvas.GetTop(_colorThumb);
			double h = ConvertBack(_converters.Value1Converter, hueY, GetValue1Range());
			double s = ConvertBack(_converters.Value2Converter, colorX, GetValue2Range());
			double v = ConvertBack(_converters.Value3Converter, colorY, GetValue3Range());
			double a = 100;
			Color = ColorHelpers.FromHSVA(h, s, v, a);
		}

		private void OnValueChange()
		{
			if(_updating == false && IsTemplateValid()) {
				_updating = true;
				UpdateThumbsFromValues();
				UpdateValuesFromThumbs();
				UpdateColorFromThumbs();
				_updating = false;
			}
		}

		private void OnColorChange()
		{
			if(_updating == false && IsTemplateValid()) {
				_updating = true;
				UpdateThumbsFromColor();
				UpdateValuesFromThumbs();
				UpdateColorFromThumbs();
				_updating = false;
			}
		}

		private void ColorCanvas_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			var position = e.GetPosition(_colorCanvas);
			_updating = true;
			MoveThumb(_colorCanvas, _colorThumb, position.X, position.Y);
			UpdateValuesFromThumbs();
			UpdateColorFromThumbs();
			_updating = false;
			_captured = true;
		}

		private void ColorCanvas_PointerReleased(object? sender, PointerReleasedEventArgs e)
		{
			if(_captured == true) {
				_captured = false;
			}
		}

		private void ColorCanvas_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(_captured == true) {
				var position = e.GetPosition(_colorCanvas);
				_updating = true;
				MoveThumb(_colorCanvas, _colorThumb, position.X, position.Y);
				UpdateValuesFromThumbs();
				UpdateColorFromThumbs();
				_updating = false;
			}
		}

		private void ColorThumb_DragDelta(object? sender, VectorEventArgs e)
		{
			if(_colorThumb == null) {
				return;
			}

			double left = Canvas.GetLeft(_colorThumb);
			double top = Canvas.GetTop(_colorThumb);
			_updating = true;
			MoveThumb(_colorCanvas, _colorThumb, left + e.Vector.X, top + e.Vector.Y);
			UpdateValuesFromThumbs();
			UpdateColorFromThumbs();
			_updating = false;
		}

		private void HueCanvas_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			var position = e.GetPosition(_hueCanvas);
			_updating = true;
			MoveThumb(_hueCanvas, _hueThumb, 0, position.Y);
			UpdateValuesFromThumbs();
			UpdateColorFromThumbs();
			_updating = false;
			_captured = true;
		}

		private void HueCanvas_PointerReleased(object? sender, PointerReleasedEventArgs e)
		{
			if(_captured == true) {
				_captured = false;
			}
		}

		private void HueCanvas_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(_captured == true) {
				var position = e.GetPosition(_hueCanvas);
				_updating = true;
				MoveThumb(_hueCanvas, _hueThumb, 0, position.Y);
				UpdateValuesFromThumbs();
				UpdateColorFromThumbs();
				_updating = false;
			}
		}

		private void HueThumb_DragDelta(object? sender, VectorEventArgs e)
		{
			if(_hueThumb == null) {
				return;
			}

			double top = Canvas.GetTop(_hueThumb);
			_updating = true;
			MoveThumb(_hueCanvas, _hueThumb, 0, top + e.Vector.Y);
			UpdateValuesFromThumbs();
			UpdateColorFromThumbs();
			_updating = false;
		}
	}
}
