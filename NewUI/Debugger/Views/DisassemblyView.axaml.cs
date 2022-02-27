using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.Views
{
	public class DisassemblyView : UserControl
	{
		private DisassemblyViewModel Model => (DisassemblyViewModel)DataContext!;
		private CpuType CpuType => Model.CpuType;

		private DisassemblyViewModel? _model;
		private CodeViewerSelectionHandler? _selectionHandler;
		private ContextMenu _bpMarginContextMenu;
		private ContextMenu _mainContextMenu;
		private DisassemblyViewer _viewer;

		static DisassemblyView()
		{
			BoundsProperty.Changed.AddClassHandler<DisassemblyView>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				int rowCount = viewer.GetVisibleRowCount();
				int prevCount = x.Model.VisibleRowCount;
				if(prevCount != rowCount) {
					x.Model.VisibleRowCount = rowCount;
					if(prevCount < rowCount) {
						x.Model.Refresh();
					}
				}
			});
		}

		public DisassemblyView()
		{
			InitializeComponent();

			_viewer = this.FindControl<DisassemblyViewer>("disViewer");
			
			InitBreakpointContextMenu();
			InitMainContextMenu();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is DisassemblyViewModel model && _model != model) {
				_model = model;
				_selectionHandler = new CodeViewerSelectionHandler(_viewer, _model, false, _mainContextMenu, _bpMarginContextMenu);
			}
			base.OnDataContextChanged(e);
		}

		[MemberNotNull(nameof(_mainContextMenu))]
		private void InitMainContextMenu()
		{
			_mainContextMenu = DebugShortcutManager.CreateContextMenu(_viewer, new List<ContextMenuAction> {
				MarkSelectionHelper.GetAction(
					() => CpuType.ToMemoryType(),
					() => Model.SelectionStart,
					() => Model.SelectionEnd,
					() => Model.Refresh()
				),
				new ContextMenuAction() {
					ActionType = ActionType.EditSelectedCode,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditSelectedCode),
					IsEnabled = () => CpuType.SupportsAssembler() && EmuApi.IsPaused(),
					OnClick = () => {
						string code = Model.GetSelection(false, false, true, false, out int byteCount);
						AssemblerWindow.EditCode(CpuType, Model.SelectionStart, code, byteCount);
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Copy,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Copy),
					OnClick = () => Model.CopySelection()
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ToggleBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_ToggleBreakpoint),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						} else if(ActionLocation.RelAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.RelAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_AddToWatch),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.RelAddress != null,
					OnClick = () => {
						if(ActionLocation.Label != null) {
							WatchManager.GetWatchManager(CpuType).AddWatch("[" + ActionLocation.Label.Label + "]");
						} else if(ActionLocation.RelAddress != null) {
							WatchManager.GetWatchManager(CpuType).AddWatch("[$" + ActionLocation.RelAddress.Value.Address.ToString(GetFormatString()) + "]");
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditLabel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditLabel),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						CodeLabel? label = ActionLocation.Label ?? (ActionLocation.AbsAddress.HasValue ? LabelManager.GetLabel(ActionLocation.AbsAddress.Value) : null);
						if(label != null) {
							LabelEditWindow.EditLabel(CpuType, this, label);
						} else if(ActionLocation.AbsAddress != null) {
							LabelEditWindow.EditLabel(CpuType, this, new CodeLabel(ActionLocation.AbsAddress.Value));
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_ViewInMemoryViewer),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						if(ActionLocation.RelAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(ActionLocation.RelAddress.Value.Type, ActionLocation.RelAddress.Value.Address);
						} else if(ActionLocation.AbsAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(ActionLocation.AbsAddress.Value.Type, ActionLocation.AbsAddress.Value.Address);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.MoveProgramCounter,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_MoveProgramCounter),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null && DebugApi.GetDebuggerFeatures(CpuType).ChangeProgramCounter,
					OnClick = () => {
						if(ActionLocation.RelAddress != null) {
							Model.Debugger.UpdateConsoleState();
							DebugApi.SetProgramCounter(CpuType, (uint)ActionLocation.RelAddress.Value.Address);
							Model.Debugger.ConsoleStatus?.UpdateUiState();
							Model.Debugger.UpdateDisassembly(true);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_GoToLocation),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.RelAddress != null,
					OnClick = () => {
						if(ActionLocation.RelAddress != null) {
							Model.SetSelectedRow(ActionLocation.RelAddress.Value.Address, true);
						}
					}
				},
			});
		}

		[MemberNotNull(nameof(_bpMarginContextMenu))]
		private void InitBreakpointContextMenu()
		{
			Breakpoint? GetBreakpoint()
			{
				return ActionLocation.AbsAddress != null ? BreakpointManager.GetMatchingBreakpoint(ActionLocation.AbsAddress.Value, CpuType) : null;
			}

			_bpMarginContextMenu = DebugShortcutManager.CreateContextMenu(_viewer, new List<ContextMenuAction> {
				new ContextMenuAction() {
					ActionType = ActionType.SetBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint() == null,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.RemoveBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint() != null,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EnableBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint()?.Enabled == false,
					IsEnabled = () => GetBreakpoint()?.Enabled == false,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.EnableDisableBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.DisableBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint()?.Enabled != false,
					IsEnabled = () => GetBreakpoint()?.Enabled == true,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.EnableDisableBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.CodeWindowEditBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => GetBreakpoint() != null,
					OnClick = () => {
						if(GetBreakpoint() is Breakpoint bp) {
							BreakpointEditWindow.EditBreakpoint(bp, this);
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

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			Model.ViewerActive = true;
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			Model.ViewerActive = false;
		}

		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
	}
}
