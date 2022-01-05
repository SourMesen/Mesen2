using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : Window
	{
		private NotificationListener? _listener = null;
		private TraceLoggerViewModel Model => ((TraceLoggerViewModel)DataContext!);
		private int _refreshCounter = 0;

		static TraceLoggerWindow()
		{
			BoundsProperty.Changed.AddClassHandler<TraceLoggerWindow>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				x.Model.VisibleRowCount = viewer.GetVisibleRowCount() - 1;
				x.Model.MaxScrollPosition = TraceLoggerViewModel.TraceLogBufferSize - x.Model.VisibleRowCount;
			});
		}

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
			DebugApi.StopLogTraceToFile();
			Model.SaveConfig();
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
						Model?.UpdateLog();
					});
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					_refreshCounter++;
					if(_refreshCounter == 6) {
						//Refresh every 6 frames, 10fps
						Dispatcher.UIThread.Post(() => {
							Model?.UpdateLog();
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

		public void Disassembly_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			Model.Scroll((int)(-e.Delta.Y * 3));
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.PageDown: Model.Scroll(Model.VisibleRowCount - 2); e.Handled = true; break;
				case Key.PageUp: Model.Scroll(-(Model.VisibleRowCount - 2)); e.Handled = true; break;
				case Key.Home: Model.ScrollToTop(); e.Handled = true; break;
				case Key.End: Model.ScrollToBottom(); e.Handled = true; break;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
