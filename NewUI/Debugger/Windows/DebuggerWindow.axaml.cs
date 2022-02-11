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
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Mesen.Debugger.Windows
{
	public class DebuggerWindow : Window, INotificationHandler
	{
		private DebuggerWindowViewModel _model;

		public CpuType CpuType => _model.CpuType;

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

			Task.Run(() => {
				//Init menu and toolbar in a separate thread to allow window to open a bit faster
				_model.InitializeMenu(this);
			});

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void RefreshDisassembly()
		{
			_model.Disassembly.Refresh();
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			//Do this in OnOpened to ensure the window is ready to receive notifications
			_model.UpdateDebugger(true);

			if(_model.Config.BreakOnOpen) {
				EmuApi.Pause();
			}
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.SaveWindowSettings(this);
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			if(_model.Disposed) {
				return;
			}

			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					BreakEvent evt = Marshal.PtrToStructure<BreakEvent>(e.Parameter);
					Dispatcher.UIThread.Post(() => {
						_model.UpdateDebugger(true, evt);
						if(_model.Config.BringToFrontOnBreak && evt.SourceCpu == _model.CpuType && evt.Source != BreakSource.PpuStep) {
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
					RomInfo romInfo = EmuApi.GetRomInfo();
					HashSet<CpuType> cpuTypes = romInfo.CpuTypes;
					if(!cpuTypes.Contains(_model.CpuType)) {
						if(!_model.IsMainCpuDebugger || _model.CpuType == romInfo.ConsoleType.GetMainCpuType()) {
							//Close the debugger window if this is not the main cpu debugger,
							//and this cpu is not supported (or it's the main cpu type for this game)
							_model.Dispose();
							Dispatcher.UIThread.Post(() => {
								Dispatcher.UIThread.RunJobs();
								Close();
							});
						} else {
							_model.Dispose();
							_model = new DebuggerWindowViewModel(null);

							Dispatcher.UIThread.Post(() => {
								_model.InitializeMenu(this);
								DataContext = _model;
							});
						}
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
