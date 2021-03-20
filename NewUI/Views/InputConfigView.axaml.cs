using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Mesen.GUI;
using Mesen.ViewModels;
using Avalonia.Interactivity;
using Mesen.Windows;

namespace Mesen.Views
{
	public class InputConfigView : UserControl
	{
		public InputConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(0, btn.Height));
			ControllerPopupWindow wnd = new ControllerPopupWindow();
			wnd.WindowStartupLocation = WindowStartupLocation.Manual;
			wnd.Position = startPosition;
			
			InputConfigViewModel model = (InputConfigViewModel)this.DataContext;
			KeyMappingSet mappings = JsonHelper.Clone(model.Config.Controllers[port].Keys);
			wnd.DataContext = mappings;

			if(await wnd.ShowDialog<bool>(this.VisualRoot as Window)) {
				model.Config.Controllers[port].Keys = mappings;
			}
		}

		private void btnSetup1_OnClick(object sender, RoutedEventArgs e)
		{
			this.OpenSetup((Button)sender, 0);
		}

		private void btnSetup2_OnClick(object sender, RoutedEventArgs e)
		{
			this.OpenSetup((Button)sender, 1);
		}

		private void btnSetup3_OnClick(object sender, RoutedEventArgs e)
		{
			this.OpenSetup((Button)sender, 2);
		}

		private void btnSetup4_OnClick(object sender, RoutedEventArgs e)
		{
			this.OpenSetup((Button)sender, 3);
		}

		private void btnSetup5_OnClick(object sender, RoutedEventArgs e)
		{
			this.OpenSetup((Button)sender, 4);
		}
	}
}
