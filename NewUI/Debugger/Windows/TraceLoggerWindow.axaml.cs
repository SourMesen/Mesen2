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
using System.IO;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : Window, INotificationHandler
	{
		private TraceLoggerViewModel _model;
		
		static TraceLoggerWindow()
		{
			BoundsProperty.Changed.AddClassHandler<TraceLoggerWindow>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				x._model.VisibleRowCount = viewer.GetVisibleRowCount() - 1;
				x._model.MaxScrollPosition = DebugApi.TraceLogBufferSize - x._model.VisibleRowCount;
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
			
			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
			DebugApi.StopLogTraceToFile();
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableTabs();
					});
					break;

				case ConsoleNotificationType.CodeBreak: {
					Dispatcher.UIThread.Post(() => {
						_model.UpdateLog();
						_model.ScrollToBottom();
					});
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					if(!ToolRefreshHelper.LimitFps(this, 10)) {
						Dispatcher.UIThread.Post(() => {
							_model.UpdateLog();
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
				_model.TraceFile = filename;
				_model.IsLoggingToFile = true;
				DebugApi.StartLogTraceToFile(filename);
			}
		}

		private void OnStopLoggingClick(object sender, RoutedEventArgs e)
		{
			if(_model.IsLoggingToFile) {
				_model.IsLoggingToFile = false;
				DebugApi.StopLogTraceToFile();
			}
		}

		private void OnOpenTraceFile(object sender, RoutedEventArgs e)
		{
			if(File.Exists(_model.TraceFile)) {
				System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo() {
					FileName = _model.TraceFile,
					UseShellExecute = true,
					Verb = "open"
				});
			}
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
