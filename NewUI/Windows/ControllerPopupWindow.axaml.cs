using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Mesen.GUI;
using Avalonia.Interactivity;

namespace Mesen.Windows
{
	public class ControllerPopupWindow : Window
	{
		public ControllerPopupWindow()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnOk_OnClick(object sender, RoutedEventArgs e)
		{
			this.Close(true);
		}

		private void btnCancel_OnClick(object sender, RoutedEventArgs e)
		{
			this.Close(false);
		}
	}
}
