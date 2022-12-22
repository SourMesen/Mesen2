using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Dock.Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.ViewModels.DebuggerDock;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.Views
{
	public class DisassemblyView : UserControl
	{
		private DisassemblyViewModel Model => _model!;
		private CpuType CpuType => Model.CpuType;
		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();

		private DisassemblyViewModel? _model;
		private CodeViewerSelectionHandler? _selectionHandler;
		private ContextMenu _bpMarginContextMenu;
		private ContextMenu _mainContextMenu;
		private DisassemblyViewer _viewer;
		private BaseToolContainerViewModel? _parentModel;

		public DisassemblyView()
		{
			InitializeComponent();

			_viewer = this.GetControl<DisassemblyViewer>("disViewer");
			_viewer.GetPropertyChangedObservable(DisassemblyViewer.VisibleRowCountProperty).Subscribe(x => {
				int rowCount = _viewer.VisibleRowCount;
				int prevCount = Model.VisibleRowCount;
				if(prevCount != rowCount) {
					Model.VisibleRowCount = rowCount;
					if(prevCount < rowCount) {
						Model.Refresh();
					}
				}
			});

			InitBreakpointContextMenu();
			InitMainContextMenu();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is DisassemblyViewModel model && _model != model) {
				_model = model;
				_selectionHandler = new CodeViewerSelectionHandler(_viewer, _model, (rowIndex, rowAddress) => rowAddress, _mainContextMenu, _bpMarginContextMenu);
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
						LocationInfo loc = ActionLocation;
						if(loc.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(loc.AbsAddress.Value, CpuType);
						} else if(loc.RelAddress != null) {
							BreakpointManager.ToggleBreakpoint(loc.RelAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_AddToWatch),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.RelAddress != null,
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						if(loc.Label != null) {
							if(loc.LabelAddressOffset != null) {
								WatchManager.GetWatchManager(CpuType).AddWatch($"[{loc.Label.Label}+{loc.LabelAddressOffset}]");
							} else {
								WatchManager.GetWatchManager(CpuType).AddWatch("[" + loc.Label.Label + "]");
							}
						} else if(loc.RelAddress != null) {
							WatchManager.GetWatchManager(CpuType).AddWatch("[$" + loc.RelAddress.Value.Address.ToString(GetFormatString()) + "]");
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditLabel,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditLabel),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						CodeLabel? label = loc.Label ?? (loc.AbsAddress.HasValue ? LabelManager.GetLabel(loc.AbsAddress.Value) : null);
						if(label != null) {
							LabelEditWindow.EditLabel(CpuType, this, label);
						} else if(loc.AbsAddress != null) {
							LabelEditWindow.EditLabel(CpuType, this, new CodeLabel(loc.AbsAddress.Value));
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_ViewInMemoryViewer),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						if(loc.RelAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(loc.RelAddress.Value.Type, loc.RelAddress.Value.Address);
						} else if(loc.AbsAddress != null) {
							MemoryToolsWindow.ShowInMemoryTools(loc.AbsAddress.Value.Type, loc.AbsAddress.Value.Address);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.FindOccurrences,
					HintText = () => GetSearchString() ?? "",
					IsEnabled = () => GetSearchString() != null,
					OnClick = () => {
						if(_model != null) {
							string? searchString = GetSearchString();
							if(searchString != null) {
								DisassemblySearchOptions options = new() { MatchWholeWord = true, MatchCase = true };
								_model.Debugger.FindAllOccurrences(searchString, options);
							}
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
						LocationInfo loc = ActionLocation;
						if(loc.RelAddress != null) {
							Model.Debugger.UpdateConsoleState();
							DebugApi.SetProgramCounter(CpuType, (uint)loc.RelAddress.Value.Address);
							Model.Debugger.ConsoleStatus?.UpdateUiState();
							Model.Debugger.UpdateDisassembly(true);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_RunToLocation),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						Model.Debugger.RunToLocation(ActionLocation);
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_GoToLocation),
					HintText = () => GetHint(ActionLocation),
					IsEnabled = () => ActionLocation.RelAddress != null,
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						if(loc.RelAddress != null) {
							Model.SetSelectedRow(loc.RelAddress.Value.Address, true);
						}
					}
				},
			});
		}

		private string? GetSearchString()
		{
			CodeSegmentInfo? segment = _selectionHandler?.MouseOverSegment;
			if(segment == null || !AllowSearch(segment.Type)) {
				LocationInfo loc = ActionLocation;
				if(loc.RelAddress?.Address >= 0) {
					CodeLabel? label = LabelManager.GetLabel(loc.RelAddress.Value);
					return label?.Label ?? ("$" + loc.RelAddress.Value.Address.ToString("X" + CpuType.GetAddressSize()));
				}
				return null;
			}
			return segment.Text.Trim(' ', '[', ']', '=', ':', '.', '+');
		}

		private bool AllowSearch(CodeSegmentType? type)
		{
			if(type == null) {
				return false;
			}

			switch(type) {
				case CodeSegmentType.OpCode:
				case CodeSegmentType.Address:
				case CodeSegmentType.Label:
				case CodeSegmentType.ImmediateValue:
				case CodeSegmentType.LabelDefinition:
				case CodeSegmentType.EffectiveAddress:
					return true;

				default:
					return false;
			}
		}

		[MemberNotNull(nameof(_bpMarginContextMenu))]
		private void InitBreakpointContextMenu()
		{
			Breakpoint? GetBreakpoint()
			{
				return ActionLocation.AbsAddress != null ? BreakpointManager.GetMatchingBreakpoint(ActionLocation.AbsAddress.Value, CpuType, true) : null;
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

		private string GetHint(LocationInfo? loc)
		{
			if(loc == null) {
				return string.Empty;
			}

			if(loc?.Label != null) {
				return loc.Label.Label + (loc.LabelAddressOffset > 0 ? ("+" + loc.LabelAddressOffset) : "");
			} else if(loc?.RelAddress != null) {
				return "$" + loc.RelAddress.Value.Address.ToString(GetFormatString());
			} else if(loc?.AbsAddress != null) {
				return "[$" + loc.AbsAddress.Value.Address.ToString(GetFormatString()) + "]";
			}

			return string.Empty;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			if(this.FindLogicalAncestorOfType<DockableControl>()?.DataContext is BaseToolContainerViewModel parentModel) {
				_parentModel = parentModel;
				_parentModel.Selected += Parent_Selected;
			}
			
			_model?.SetViewer(_viewer);
			_viewer.Focus();
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			if(_parentModel != null) {
				_parentModel.Selected -= Parent_Selected;
				_parentModel = null;
			}

			_model?.SetViewer(null);
		}

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);

			//Navigate on double-click left click
			if(_selectionHandler?.IsMarginClick == false && ActionLocation.RelAddress != null && e.GetCurrentPoint(this).Properties.IsLeftButtonPressed && e.ClickCount == 2) {
				Model.SetSelectedRow(ActionLocation.RelAddress.Value.Address, true);
			}
		}

		private void Parent_Selected(object? sender, EventArgs e)
		{
			Dispatcher.UIThread.Post(() => {
				//Focus disassembly view when selected by code
				_viewer.Focus();
			});
		}
	}
}
