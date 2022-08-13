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
	public class TraceLoggerWindow : Window, INotificationHandler
	{
		private TraceLoggerViewModel _model;
		private CodeViewerSelectionHandler _selectionHandler;

		static TraceLoggerWindow()
		{
			BoundsProperty.Changed.AddClassHandler<TraceLoggerWindow>((x, e) => {
				DisassemblyViewer viewer = x.GetControl<DisassemblyViewer>("disViewer");
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
			_model.InitializeMenu(this);
			
			DisassemblyViewer viewer = this.GetControl<DisassemblyViewer>("disViewer");
			_model.SetViewer(viewer);
			_selectionHandler = new CodeViewerSelectionHandler(viewer, _model, (rowIndex, rowAddress) => rowIndex, InitContextMenu(viewer));

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

		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
		private CpuType CpuType => ActionLocation?.RelAddress?.Type.ToCpuType() ?? ActionLocation?.AbsAddress?.Type.ToCpuType() ?? _model.SelectedTab.CpuType;

		private ContextMenu InitContextMenu(DisassemblyViewer viewer)
		{
			return DebugShortcutManager.CreateContextMenu(viewer, new List<ContextMenuAction> {
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
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.AddBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						} else if(ActionLocation.RelAddress != null) {
							BreakpointManager.AddBreakpoint(ActionLocation.RelAddress.Value, CpuType);
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
			});
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
					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableTabs();
					});
					break;

				case ConsoleNotificationType.CodeBreak: {
					if(_model.Config.RefreshOnBreakPause) {
						Dispatcher.UIThread.Post(() => {
							_model.UpdateLog();
							_model.ScrollToBottom();
						});
					}
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					if(_model.Config.AutoRefresh && !ToolRefreshHelper.LimitFps(this, 10)) {
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
