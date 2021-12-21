using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;

namespace Mesen.Debugger.Controls
{
	public class ColorCheckbox : UserControl
	{
		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<ColorCheckbox, string>(nameof(Text), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<bool> CheckedProperty = AvaloniaProperty.Register<ColorCheckbox, bool>(nameof(Checked), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<Color> ColorProperty = AvaloniaProperty.Register<ColorCheckbox, Color>(nameof(Color), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}

		public bool Checked
		{
			get { return GetValue(CheckedProperty); }
			set { SetValue(CheckedProperty, value); }
		}

		public Color Color
		{
			get { return GetValue(ColorProperty); }
			set { SetValue(ColorProperty, value); }
		}

		public ColorCheckbox()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private async void OnColorClick(object sender, RoutedEventArgs e)
		{
			ColorPickerViewModel model = new ColorPickerViewModel() { Color = Color };
			ColorPickerWindow wnd = new ColorPickerWindow() { DataContext = model };

			bool success = await wnd.ShowCenteredDialog<bool>(this);
			if(success) {
				Color = model.Color;
			}
		}
	}
}
