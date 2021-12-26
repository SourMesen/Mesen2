#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Mesen.Config;

namespace Mesen.Debugger.Windows
{
	public class EventViewerWindow : Window
	{
		private NotificationListener _listener;
		private EventViewerViewModel _model;
		private DispatcherTimer _timer;

		//For designer
		[Obsolete] public EventViewerWindow() : this(CpuType.Cpu) { }

		public EventViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif

			_model = new EventViewerViewModel(cpuType, this.FindControl<PictureViewer>("picViewer"), this);
			_model.Config.LoadWindowSettings(this);
			DataContext = _model;

			if(Design.IsDesignMode) {
				return;
			}

			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateConfig());
			_listener = new NotificationListener();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_timer.Start();
			_listener.OnNotification += listener_OnNotification;
			_model.RefreshViewer();
		}

		private void UpdateConfig()
		{
			if(_model.ConsoleConfig is SnesEventViewerConfig snesCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, snesCfg.ToInterop());
			} else if(_model.ConsoleConfig is NesEventViewerConfig nesCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, nesCfg.ToInterop());
			} else if(_model.ConsoleConfig is GbEventViewerConfig gbCfg) {
				DebugApi.SetEventViewerConfig(_model.CpuType, gbCfg.ToInterop());
			}
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_model.Config.SaveWindowSettings(this);
			_timer.Stop();
			_listener.Dispose();
			_model.SaveConfig();
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.EventViewerRefresh:
					if(_model.Config.AutoRefresh) {
						_model.RefreshViewer();
					}
					break;

				case ConsoleNotificationType.CodeBreak:
					if(_model.Config.RefreshOnBreakPause) {
						_model.RefreshViewer();
					}
					break;
			}
		}
	}
}
