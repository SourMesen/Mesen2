using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Avalonia.Interactivity;
using System.ComponentModel;
using Avalonia.Threading;
using Mesen.Config;
using System.Runtime.InteropServices;
using Mesen.Debugger.Utilities;
using Mesen.Utilities;

namespace Mesen.Debugger.Windows
{
	public class DebuggerWindow : Window, INotificationHandler
	{
		private DebuggerWindowViewModel _model;
		
		[Obsolete("For designer only")]
		public DebuggerWindow() : this(null) { }

		public DebuggerWindow(CpuType? cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = new DebuggerWindowViewModel(cpuType);
			DataContext = _model;

			_model.InitializeMenu(this);

			if(Design.IsDesignMode) {
				return;
			}

			if(_model.Config.BreakOnOpen) {
				EmuApi.Pause();
			}

			_model.UpdateDebugger(true);
			_model.Config.LoadWindowSettings(this);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			if(Design.IsDesignMode) {
				return;
			}

			_model.Dispose();
			_model.Config.SaveWindowSettings(this);
			ConfigManager.SaveConfig();
			DataContext = null;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					BreakEvent evt = Marshal.PtrToStructure<BreakEvent>(e.Parameter);
					Dispatcher.UIThread.Post(() => {
						_model.UpdateDebugger(true, evt);
						if(_model.Config.BringToFrontOnBreak) {
							Activate();
						}
					});
					break;

				case ConsoleNotificationType.DebuggerResumed:
					_model.UpdateConsoleState();
					Dispatcher.UIThread.Post(() => {
						_model.ProcessResumeEvent();
					});
					break;

				case ConsoleNotificationType.GameReset:
					if(_model.Config.BreakOnPowerCycleReset) {
						EmuApi.Pause();
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					if(!EmuApi.GetRomInfo().CpuTypes.Contains(_model.CpuType)) {
						_model.Dispose();
						_model = new DebuggerWindowViewModel(null);
						
						Dispatcher.UIThread.Post(() => {
							_model.InitializeMenu(this);
							DataContext = _model;
						});
					}

					if(_model.Config.BreakOnPowerCycleReset) {
						EmuApi.Pause();
					}
					break;
			}
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}
	}
}
