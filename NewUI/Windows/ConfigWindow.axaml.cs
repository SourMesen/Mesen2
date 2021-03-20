using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.GUI;
using ReactiveUI;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.ComponentModel;
using Mesen.GUI.Config;

namespace Mesen.Windows
{
	public class ConfigWindow : Window
	{
		private DispatcherTimer _timer;
		public ConfigWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(200), DispatcherPriority.Normal, (s, e) => (this.DataContext as ConfigViewModel).ApplyConfig());
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			_timer.Start();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			(this.DataContext as ConfigViewModel)?.SaveConfig();
			this.Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			this.Close();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_timer.Stop();
			ConfigManager.Config.ApplyConfig();
		}
	}
}