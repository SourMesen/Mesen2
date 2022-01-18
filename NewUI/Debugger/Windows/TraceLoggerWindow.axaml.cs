using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : Window
	{
		private TraceLoggerViewModel _model;
		private NotificationListener _listener;
		
		static TraceLoggerWindow()
		{
			BoundsProperty.Changed.AddClassHandler<TraceLoggerWindow>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				x._model.VisibleRowCount = viewer.GetVisibleRowCount() - 1;
				x._model.MaxScrollPosition = TraceLoggerViewModel.TraceLogBufferSize - x._model.VisibleRowCount;
			});
		}

		[Obsolete("For designer only")]
		public TraceLoggerWindow() : this(new()) { }

		public TraceLoggerWindow(TraceLoggerViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			DataContext = model;
			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
			
			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
			_listener?.Dispose();
			DebugApi.StopLogTraceToFile();
			_model.SaveConfig();
			_model.Dispose();
			DataContext = null;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.EmulationStopped:
					Dispatcher.UIThread.Post(() => {
						Close();
					});
					break;

				case ConsoleNotificationType.GameLoaded:
					_model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.CodeBreak: {
					_model.UpdateLog();
					Dispatcher.UIThread.Post(() => {
						_model.ScrollToBottom();
					});
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					if(ToolRefreshHelper.LimitFps(this, 10)) {
						_model.UpdateLog();
						Dispatcher.UIThread.Post(() => {
							_model.ScrollToBottom();
						});
					}
					break;
				}
			}
		}

		private async void OnStartLoggingClick(object sender, RoutedEventArgs e)
		{
			string? filename = await FileDialogHelper.SaveFile(ConfigManager.DebuggerFolder, EmuApi.GetRomInfo().GetRomName() + ".txt", VisualRoot, FileDialogHelper.TraceExt);
			if(filename != null) {
				_model.IsLoggingToFile = true;
				DebugApi.StartLogTraceToFile(filename);
			}
		}

		private void OnStopLoggingClick(object sender, RoutedEventArgs e)
		{
			_model.IsLoggingToFile = false;
			DebugApi.StopLogTraceToFile();
		}

		public void Disassembly_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			_model.Scroll((int)(-e.Delta.Y * 3));
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.PageDown: _model.Scroll(_model.VisibleRowCount - 2); e.Handled = true; break;
				case Key.PageUp: _model.Scroll(-(_model.VisibleRowCount - 2)); e.Handled = true; break;
				case Key.Home: _model.ScrollToTop(); e.Handled = true; break;
				case Key.End: _model.ScrollToBottom(); e.Handled = true; break;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
