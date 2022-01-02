using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : Window
	{
		private NotificationListener? _listener = null;
		private TraceLoggerViewModel Model => ((TraceLoggerViewModel)DataContext!);
		private int _refreshCounter = 0;

		public TraceLoggerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_listener?.Dispose();
			DataContext = null;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.CodeBreak: {
					Dispatcher.UIThread.Post(() => {
						Model.UpdateLog();
					});
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					_refreshCounter++;
					if(_refreshCounter == 9) {
						//Refresh every 9 frames, ~7fps
						Dispatcher.UIThread.Post(() => {
							Model.UpdateLog();
						});
						_refreshCounter = 0;
					}
					break;
				}
			}
		}

		private async void OnStartLoggingClick(object sender, RoutedEventArgs e)
		{
			string? filename = await FileDialogHelper.SaveFile(ConfigManager.DebuggerFolder, EmuApi.GetRomInfo().GetRomName() + ".txt", VisualRoot, FileDialogHelper.TraceExt);
			if(filename != null) {
				Model.IsLoggingToFile = true;
				DebugApi.StartLogTraceToFile(filename);
			}
		}

		private void OnStopLoggingClick(object sender, RoutedEventArgs e)
		{
			Model.IsLoggingToFile = false;
			DebugApi.StopLogTraceToFile();
		}

		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
			DebugApi.StopLogTraceToFile();
			Model.SaveConfig();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
