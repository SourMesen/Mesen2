using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Dock.Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Integration;
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
	public class SourceViewView : UserControl
	{
		private SourceViewViewModel Model => (SourceViewViewModel)DataContext!;
		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
		private CpuType CpuType => Model.CpuType;

		private SourceViewViewModel? _model;
		private CodeViewerSelectionHandler? _selectionHandler;
		private ContextMenu _bpMarginContextMenu;
		private ContextMenu _mainContextMenu;
		private DisassemblyViewer _viewer;
		private BaseToolContainerViewModel? _parentModel;

		static SourceViewView()
		{
			BoundsProperty.Changed.AddClassHandler<SourceViewView>((x, e) => {
				int rowCount = x._viewer.GetVisibleRowCount();
				int prevCount = x.Model.VisibleRowCount;
				if(prevCount != rowCount) {
					int pos = x.Model.ScrollPosition + (prevCount / 2);
					x.Model.VisibleRowCount = rowCount;
					x.Model.ScrollToRowNumber(pos);
					x.Model.Refresh();
				}
			});
		}

		public SourceViewView()
		{
			InitializeComponent();
			_viewer = this.FindControl<DisassemblyViewer>("disViewer");

			InitBreakpointContextMenu();
			InitMainContextMenu();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is SourceViewViewModel model && _model != model) {
				_model = model;
				_selectionHandler = new CodeViewerSelectionHandler(_viewer, _model, (rowIndex, rowAddress) => rowIndex + _model.ScrollPosition, _mainContextMenu, _bpMarginContextMenu);
			}
			base.OnDataContextChanged(e);
		}


		[MemberNotNull(nameof(_mainContextMenu))]
		private void InitMainContextMenu()
		{
			_mainContextMenu = DebugShortcutManager.CreateContextMenu(_viewer, new List<ContextMenuAction> {
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
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
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
					IsEnabled = () => ActionLocation.Symbol != null || ActionLocation.RelAddress != null,
					OnClick = () => {
						if(ActionLocation.Symbol != null) {
							AddressInfo? addr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(ActionLocation.Symbol.Value);
							if(addr?.Address > 0) {
								SourceCodeLocation? loc = DebugWorkspaceManager.SymbolProvider?.GetSourceCodeLineInfo(addr.Value);
								if(loc != null) {
									_model?.ScrollToLocation(loc.Value);
								}
							}
						} else if(ActionLocation.RelAddress != null) {
							_model?.GoToRelativeAddress(ActionLocation.RelAddress.Value.Address);
						}
					}
				},
			});
		}

		private string? GetSearchString()
		{
			CodeSegmentInfo? segment = _selectionHandler?.MouseOverSegment;
			if(segment == null || !AllowSearch(segment.Type)) {
				if(ActionLocation.RelAddress?.Address >= 0) {
					CodeLabel? label = LabelManager.GetLabel(ActionLocation.RelAddress.Value);
					return label?.Label ?? ("$" + ActionLocation.RelAddress.Value.Address.ToString("X" + CpuType.GetAddressSize()));
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

			if(codeLoc?.Symbol != null) {
				return codeLoc.Symbol.Value.Name;
			} else if(codeLoc?.RelAddress != null) {
				return "$" + codeLoc.RelAddress.Value.Address.ToString(GetFormatString());
			} else if(codeLoc?.AbsAddress != null) {
				return "[$" + codeLoc.AbsAddress.Value.Address.ToString(GetFormatString()) + "]";
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

		private void Parent_Selected(object? sender, EventArgs e)
		{
			Dispatcher.UIThread.Post(() => {
				//Focus source view when selected by code
				_viewer.Focus();
			});
		}
	}
}
