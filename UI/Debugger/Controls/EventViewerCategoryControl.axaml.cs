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
	public class EventViewerCategoryControl : UserControl
	{
		public static readonly StyledProperty<string> TextProperty = AvaloniaProperty.Register<EventViewerCategoryControl, string>(nameof(Text), "", defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<EventViewerCategoryCfg> ConfigProperty = AvaloniaProperty.Register<EventViewerCategoryControl, EventViewerCategoryCfg>(nameof(Config), new EventViewerCategoryCfg(), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

		public string Text
		{
			get { return GetValue(TextProperty); }
			set { SetValue(TextProperty, value); }
		}

		public EventViewerCategoryCfg Config
		{
			get { return GetValue(ConfigProperty); }
			set { SetValue(ConfigProperty, value); }
		}

		public EventViewerCategoryControl()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private async void OnColorClick(object sender, RoutedEventArgs e)
		{
			ColorPickerViewModel model = new ColorPickerViewModel() { Color = Color.FromUInt32(Config.Color) };
			ColorPickerWindow wnd = new ColorPickerWindow() { DataContext = model };

			bool success = await wnd.ShowCenteredDialog<bool>(this);
			if(success) {
				Config.Color = model.Color.ToUInt32();
			}
		}
	}
}
