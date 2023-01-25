using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using System;

namespace Mesen.Controls
{
	public class MesenSlider : UserControl
	{
		public static readonly StyledProperty<int> MinimumProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Minimum));
		public static readonly StyledProperty<int> MaximumProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Maximum));
		public static readonly StyledProperty<int> ValueProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(Value), 0, false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<MesenSlider, string>(nameof(Text), "");
		public static readonly StyledProperty<bool> HideValueProperty = AvaloniaProperty.Register<MesenSlider, bool>(nameof(HideValue));
		public static readonly StyledProperty<int> TickFrequencyProperty = AvaloniaProperty.Register<MesenSlider, int>(nameof(TickFrequency), 10);
		public static readonly StyledProperty<Orientation> OrientationProperty = AvaloniaProperty.Register<MesenSlider, Orientation>(nameof(Orientation));

		public int Minimum
		{
			get { return GetValue(MinimumProperty); }
			set { SetValue(MinimumProperty, value); }
		}

		public int Maximum
		{
			get { return GetValue(MaximumProperty); }
			set { SetValue(MaximumProperty, value); }
		}

		public int Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}
		
		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}
		
		public bool HideValue
		{
			get { return GetValue(HideValueProperty); }
			set { SetValue(HideValueProperty, value); }
		}

		public int TickFrequency
		{
			get { return GetValue(TickFrequencyProperty); }
			set { SetValue(TickFrequencyProperty, value); }
		}

		public Orientation Orientation
		{
			get { return GetValue(OrientationProperty); }
			set { SetValue(OrientationProperty, value); }
		}

		public MesenSlider()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void Slider_OnPropertyChanged(object? sender, AvaloniaPropertyChangedEventArgs e)
		{
			if(e.Property == Slider.ValueProperty && e.NewValue is double value && sender is Slider slider) {
				double newIntegerValue = Math.Floor(value);
				if(newIntegerValue != (double)e.NewValue) {
					slider.Value = newIntegerValue;
				} else {
					slider.Value = Math.Ceiling(value);
				}
			}
		}
	}
}
