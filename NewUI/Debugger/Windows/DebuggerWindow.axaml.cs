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

			ReactiveHelper.RegisterRecursiveObserver(_model.Config, Config_PropertyChanged);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			_model.Config.ApplyConfig();

			Dispatcher.UIThread.Post(() => {
				_model.UpdateDebugger();
			});
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			if(Design.IsDesignMode) {
				return;
			}

			DetachDebugger();
			_model.Config.SaveWindowSettings(this);
			ConfigManager.SaveConfig();
			//DataContext = null;
		}

		private void DetachDebugger()
		{
			ReactiveHelper.UnregisterRecursiveObserver(_model.Config, Config_PropertyChanged);
			_model.Cleanup();
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
						DebugWindowManager.PreventNotifications(this);
						DetachDebugger();
						Dispatcher.UIThread.Post(() => {
							Close();
						});
					} else {
						if(_model.Config.BreakOnPowerCycleReset) {
							EmuApi.Pause();
						}
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
