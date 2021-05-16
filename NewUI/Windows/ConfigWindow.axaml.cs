using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using Mesen.Config;

namespace Mesen.Windows
{
	public class ConfigWindow : Window
	{
		private ConfigViewModel? _model;
		private DispatcherTimer _timer;

		public ConfigWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(200), DispatcherPriority.Normal, (s, e) => _model?.ApplyConfig());
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is ConfigViewModel model) {
				_model = model;
			} else {
				throw new Exception("Invalid model");
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_timer?.Start();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_model?.SaveConfig();
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_timer.Stop();
			ConfigManager.Config.ApplyConfig();
		}
	}
}