using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Metadata;
using System;

namespace Mesen.Debugger.Controls
{
	public class HexInput : UserControl
	{
		public static readonly StyledProperty<int> ValueProperty = AvaloniaProperty.Register<HexInput, int>(nameof(Value), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<int?> MaximumProperty = AvaloniaProperty.Register<HexInput, int?>(nameof(Maximum));
		public static readonly StyledProperty<int?> MinimumProperty = AvaloniaProperty.Register<HexInput, int?>(nameof(Minimum));
		public static readonly StyledProperty<int> SmallIncrementProperty = AvaloniaProperty.Register<HexInput, int>(nameof(SmallIncrement));
		public static readonly StyledProperty<int> LargeIncrementProperty = AvaloniaProperty.Register<HexInput, int>(nameof(LargeIncrement));

		public int Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public int? Minimum
		{
			get { return GetValue(MinimumProperty); }
			set { SetValue(MinimumProperty, value); }
		}

		public int? Maximum
		{
			get { return GetValue(MaximumProperty); }
			set { SetValue(MaximumProperty, value); }
		}

		public int SmallIncrement
		{
			get { return GetValue(SmallIncrementProperty); }
			set { SetValue(SmallIncrementProperty, value); }
		}

		public int LargeIncrement
		{
			get { return GetValue(LargeIncrementProperty); }
			set { SetValue(LargeIncrementProperty, value); }
		}

		static HexInput()
		{
		}

		public HexInput()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void SetValue(int offset)
		{
			int value = Value + offset;
			Value = value;
		}

		private void OnNextLargeClick(object sender, RoutedEventArgs e)
		{
			SetValue(LargeIncrement);
		}

		private void OnNextSmallClick(object sender, RoutedEventArgs e)
		{
			SetValue(SmallIncrement);
		}

		public void DecrementLarge(object parameter)
		{
			SetValue(-LargeIncrement);
		}

		[DependsOn(nameof(Value))]
		public bool CanDecrementLarge(object parameter)
		{
			return Value > 0;
		}

		public void DecrementSmall(object parameter)
		{
			SetValue(-SmallIncrement);
		}

		[DependsOn(nameof(Value))]
		public bool CanDecrementSmall(object parameter)
		{
			return Value > 0;
		}

		public void IncrementLarge(object parameter)
		{
			SetValue(LargeIncrement);
		}

		[DependsOn(nameof(Value))]
		[DependsOn(nameof(Maximum))]
		[DependsOn(nameof(LargeIncrement))]
		public bool CanIncrementLarge(object parameter)
		{
			return Value < Maximum && Value < Maximum - LargeIncrement + 1;
		}

		public void IncrementSmall(object parameter)
		{
			SetValue(SmallIncrement);
		}

		[DependsOn(nameof(Value))]
		[DependsOn(nameof(Maximum))]
		public bool CanIncrementSmall(object parameter)
		{
			return Value < Maximum;
		}
	}
}
