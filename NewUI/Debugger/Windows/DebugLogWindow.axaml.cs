using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using System.ComponentModel;
using Avalonia.Data;
using Mesen.Interop;
using Mesen.Config;

namespace Mesen.Debugger.Windows
{
	public class DebugLogWindow : Window
	{
		private DispatcherTimer _timer;

		public static readonly StyledProperty<string> LogContentProperty = AvaloniaProperty.Register<DebugLogWindow, string>(nameof(LogContent), "", defaultBindingMode: BindingMode.OneWayToSource);

		public string LogContent
		{
			get { return GetValue(LogContentProperty); }
			set { SetValue(LogContentProperty, value); }
		}

		public DebugLogWindow()
		{
			InitializeComponent();
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateLog());

			ConfigManager.Config.Debug.DebugLog.LoadWindowSettings(this);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void UpdateLog()
		{
			string newLog = DebugApi.GetLog();
			if(newLog != LogContent) {
				LogContent = newLog;
				this.FindControl<TextBox>("txtLog").CaretIndex = Int32.MaxValue;
			}
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_timer.Start();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}
			_timer.Stop();
			ConfigManager.Config.Debug.DebugLog.SaveWindowSettings(this);
			DataContext = null;
		}
	}
}