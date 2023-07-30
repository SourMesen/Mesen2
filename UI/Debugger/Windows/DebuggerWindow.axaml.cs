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
using Avalonia.VisualTree;
using Avalonia.Input;
using Mesen.Debugger.Views;
using System.Linq;
using System.IO;

namespace Mesen.Debugger.Windows
{
	public class DebuggerWindow : MesenWindow, INotificationHandler
	{
		private DebuggerWindowViewModel _model;

		public CpuType CpuType => _model.CpuType;
		private int? _scrollToAddress = null;
		private bool _suppressBringToFront = false;
		private bool _autoResumePending = false;

		[Obsolete("For designer only")]
		public DebuggerWindow() : this(null, null) { }

		public DebuggerWindow(CpuType? cpuType, int? scrollToAddress = null)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = new DebuggerWindowViewModel(cpuType);
			_scrollToAddress = scrollToAddress;
			DataContext = _model;

			_model.InitializeMenu(this);

			if(Design.IsDesignMode) {
				return;
			}
			
			AddHandler(DragDrop.DropEvent, OnDrop);

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

		public void ScrollToAddress(uint address)
		{
			_model.ScrollToAddress((int)address);
		}

		public static DebuggerWindow GetOrOpenWindow(CpuType cpuType)
		{
			DebuggerWindow? wnd = DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == cpuType);
			if(wnd == null) {
				return DebugWindowManager.OpenDebugWindow<DebuggerWindow>(() => new DebuggerWindow(cpuType));
			} else {
				wnd.BringToFront();
			}
			return wnd;
		}

		public static void OpenWindowAtAddress(CpuType cpuType, int address)
		{
			DebuggerWindow? debugger = DebugWindowManager.GetDebugWindow<DebuggerWindow>(wnd => wnd.CpuType == cpuType);
			if(debugger == null) {
				debugger = DebugWindowManager.OpenDebugWindow<DebuggerWindow>(() => new DebuggerWindow(cpuType, address));
			} else {
				debugger.ScrollToAddress((uint)address);
			}
			debugger.BringToFront();
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			//Do this in OnOpened to ensure the window is ready to receive notifications
			Dispatcher.UIThread.Post(() => {
				_model.Init();
				_model.UpdateDebugger(true);

				if(_scrollToAddress.HasValue) {
					ScrollToAddress((uint)_scrollToAddress);
				} else if(_model.Config.BreakOnOpen) {
					if(!EmuApi.IsPaused()) {
						_model.Step(StepType.Step);
					}
				}
			});
		}

		protected override void OnClosing(WindowClosingEventArgs e)
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
						BreakpointManager.ClearTemporaryBreakpoints();

						_model.UpdateDebugger(true, evt);
						if(!_suppressBringToFront) {
							bool isPause = evt.Source == BreakSource.Pause;
							bool isBreak = !isPause && evt.Source != BreakSource.PpuStep && evt.Source != BreakSource.InternalOperation;
							if(isPause && _model.Config.BringToFrontOnPause && evt.SourceCpu == _model.CpuType) {
								Activate();
							} else if(isBreak && _model.Config.BringToFrontOnBreak && evt.SourceCpu == _model.CpuType) {
								Activate();
							}
						}
						_suppressBringToFront = false;

						if(_autoResumePending) {
							EmuApi.Resume();
							_autoResumePending = false;
						}
					});
					break;

				case ConsoleNotificationType.DebuggerResumed:
					_model.UpdateConsoleState();
					Dispatcher.UIThread.Post(() => {
						_model.ProcessResumeEvent(this);
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
					} else {
						_model.ControllerList.SetInputOverrides();
					}

					GameLoadedEventParams evtParams = Marshal.PtrToStructure<GameLoadedEventParams>(e.Parameter);
					if(!evtParams.IsPaused) {
						//If not already paused, pause on load to ensure UI can load labels, breakpoints, etc.
						EmuApi.Pause();

						if(!_model.Config.BreakOnPowerCycleReset) {
							//If the option to break on power cycle is turned off, auto-resume after the first break is processed
							_autoResumePending = true;
						}
					}
					break;


				case ConsoleNotificationType.PpuFrameDone:
					if(_model.Config.RefreshWhileRunning) {
						Dispatcher.UIThread.Post(() => {
							if(!ToolRefreshHelper.LimitFps(this, 20)) {
								//Prevent watch update when user is typing a new watch entry
								bool updatingWatchEntry = TopLevel.GetTopLevel(this)?.FocusManager?.GetFocusedElement() is TextBox txt && txt.FindAncestorOfType<WatchListView>() != null;
								_model.PartialRefresh(!updatingWatchEntry);
							}
						});
					}
					break;

				case ConsoleNotificationType.StateLoaded:
					Dispatcher.UIThread.Post(() => {
						//Update UI after loading a state (to update highlighted statement, etc.)
						_model.UpdateDebugger(true, null);
					});
					break;
			}
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void OnDrop(object? sender, DragEventArgs e)
		{
			string? filename = e.Data.GetFiles()?.FirstOrDefault()?.Path.LocalPath;
			if(filename != null && File.Exists(filename)) {
				Activate();
				DebugWorkspaceManager.LoadSupportedFile(filename, true);
			}
		}

		public void SuppressBringToFront()
		{
			//Used to prevent debugger from stealing focus when emulation is
			//paused by the "Pause in menus and config windows" option
			_suppressBringToFront = true;
		}
	}
}
