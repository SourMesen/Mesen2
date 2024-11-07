using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : MesenWindow, INotificationHandler
	{
		private TraceLoggerViewModel _model;
		private CodeViewerSelectionHandler _selectionHandler;

		[Obsolete("For designer only")]
		public TraceLoggerWindow() : this(new()) { }

		public TraceLoggerWindow(TraceLoggerViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			_model.InitializeMenu(this);
			
			DisassemblyViewer viewer = this.GetControl<DisassemblyViewer>("disViewer");
			_model.SetViewer(viewer);
			InitContextMenu(viewer);
			_selectionHandler = new CodeViewerSelectionHandler(viewer, _model, (rowIndex, rowAddress) => rowIndex, false);

			viewer.GetPropertyChangedObservable(DisassemblyViewer.VisibleRowCountProperty).Subscribe(x => {
				_model.VisibleRowCount = Math.Max(1, viewer.VisibleRowCount - 1);
				_model.MaxScrollPosition = DebugApi.TraceLogBufferSize - _model.VisibleRowCount;
			});

			DataContext = model;

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
			DebugApi.StopLogTraceToFile();
			
			//Disable trace logging for all cpus
			foreach(CpuType cpuType in Enum.GetValues<CpuType>()) {
				DebugApi.SetTraceOptions(cpuType, new());
			}
		}

		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
		private CpuType CpuType => ActionLocation?.RelAddress?.Type.ToCpuType() ?? ActionLocation?.AbsAddress?.Type.ToCpuType() ?? _model.SelectedTab.CpuType;

		private void InitContextMenu(DisassemblyViewer viewer)
		{
			_model.AddDisposables(DebugShortcutManager.CreateContextMenu(viewer, new List<ContextMenuAction> {
				new ContextMenuAction() {
					ActionType = ActionType.Copy,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Copy),
					OnClick = () => _model.CopySelection()
				},
				new ContextMenuAction() {
					ActionType = ActionType.SelectAll,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SelectAll),
					OnClick = () => _model.SelectAll()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EditBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TraceLogger_EditBreakpoint),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						AddressInfo? addr = ActionLocation.AbsAddress ?? ActionLocation.RelAddress;
						if(addr != null) {
							Breakpoint bp = new Breakpoint() {
								BreakOnRead = true, BreakOnWrite = true, BreakOnExec = true,
								MemoryType = addr.Value.Type,
								StartAddress = (uint)addr.Value.Address,
								EndAddress = (uint)addr.Value.Address,
								CpuType = CpuType
							};
							BreakpointEditWindow.EditBreakpoint(bp, viewer);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditLabel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TraceLogger_EditLabel),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						CodeLabel? label = ActionLocation.Label ?? (ActionLocation.AbsAddress.HasValue ? LabelManager.GetLabel(ActionLocation.AbsAddress.Value) : null);
						if(label != null) {
							LabelEditWindow.EditLabel(CpuType, this, label);
						} else if(ActionLocation.AbsAddress != null) {
							LabelEditWindow.EditLabel(CpuType, this, new CodeLabel(ActionLocation.AbsAddress.Value));
						}
						_model.InvalidateVisual();
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ViewInDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TraceLogger_ViewInDebugger),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null,
					OnClick = () => {
						if(ActionLocation.RelAddress != null) {
							DebuggerWindow.OpenWindowAtAddress(CpuType, ActionLocation.RelAddress.Value.Address);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TraceLogger_ViewInMemoryViewer),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						if(ActionLocation.RelAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(ActionLocation.RelAddress.Value.Type, ActionLocation.RelAddress.Value.Address);
						} else if(ActionLocation.AbsAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(ActionLocation.AbsAddress.Value.Type, ActionLocation.AbsAddress.Value.Address);
						}
					}
				}
			}));
		}

		private string GetFormatString()
		{
			return CpuType.ToMemoryType().GetFormatString();
		}

		private string GetHint(LocationInfo? codeLoc)
		{
			if(codeLoc == null) {
				return string.Empty;
			}

			if(codeLoc?.RelAddress != null) {
				return "$" + codeLoc.RelAddress.Value.Address.ToString(GetFormatString());
			}

			return string.Empty;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					_model.UpdateCoreOptions();

					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableTabs();
					});
					break;

				case ConsoleNotificationType.CodeBreak: {
					if(_model.Config.RefreshOnBreakPause) {
						_model.UpdateLog(true);
					}
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					if(_model.Config.AutoRefresh && !ToolRefreshHelper.LimitFps(this, 10)) {
						_model.UpdateLog(true);
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

		private void OnClearClick(object sender, RoutedEventArgs e)
		{
			DebugApi.ClearExecutionTrace();
			_model.UpdateLog();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
