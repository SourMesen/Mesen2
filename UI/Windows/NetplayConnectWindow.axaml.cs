using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System.Collections.Generic;

namespace Mesen.Windows
{
	public class NetplayConnectWindow : MesenWindow
	{
		public NetplayConnectWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			NetplayConfig cfg = (NetplayConfig)DataContext!;
			ConfigManager.Config.Netplay = cfg.Clone();

			Close(true);

			NetplayApi.Connect(cfg.Host, cfg.Port, cfg.Password, false); 
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}