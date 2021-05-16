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
using Avalonia.Data;
using Mesen.Interop;

namespace Mesen.Windows
{
	public class LogWindow : Window
	{
		private DispatcherTimer _timer;

		public static readonly StyledProperty<string> LogContentProperty = AvaloniaProperty.Register<LogWindow, string>(nameof(LogContent), "", defaultBindingMode: BindingMode.OneWayToSource);

		public string LogContent
		{
			get { return GetValue(LogContentProperty); }
			set { SetValue(LogContentProperty, value); }
		}

		public LogWindow()
		{
			InitializeComponent();
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateLog());
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void UpdateLog()
		{
			string newLog = EmuApi.GetLog();
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
		}
	}
}