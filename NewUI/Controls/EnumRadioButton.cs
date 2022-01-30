using Avalonia;
using Avalonia.Controls;
using System;
using System.Linq;
using Avalonia.Styling;
using Avalonia.Data;
using Avalonia.Interactivity;

namespace Mesen.Controls
{
	public class EnumRadioButton : RadioButton, IStyleable
	{
		Type IStyleable.StyleKey => typeof(RadioButton);

		public static readonly StyledProperty<Enum> ValueProperty = AvaloniaProperty.Register<EnumRadioButton, Enum>(nameof(Value), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<Enum> CheckedWhenProperty = AvaloniaProperty.Register<EnumRadioButton, Enum>(nameof(CheckedWhen));

		public Enum Value
		{
			get { return GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		public Enum CheckedWhen
		{
			get { return GetValue(CheckedWhenProperty); }
			set { SetValue(CheckedWhenProperty, value); }
		}

		static EnumRadioButton()
		{
			ValueProperty.Changed.AddClassHandler<EnumRadioButton>((x, e) => {
				x.IsChecked = x.Value.Equals(x.CheckedWhen);
			});

			CheckedWhenProperty.Changed.AddClassHandler<EnumRadioButton>((x, e) => {
				x.IsChecked = x.Value.Equals(x.CheckedWhen);
			});
		}

		public EnumRadioButton()
		{
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			IsChecked = Value.Equals(CheckedWhen);
		}

		protected override void OnChecked(RoutedEventArgs e)
		{
			base.OnChecked(e);
			Value = CheckedWhen;
		}
	}
}
