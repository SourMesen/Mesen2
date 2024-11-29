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
using Mesen.Utilities;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Views
{
	public class DisassemblyView : MesenUserControl
	{
		private DisassemblyViewModel Model => _model!;
		private CpuType CpuType => Model.CpuType;
		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
		private bool IsMarginClick => _selectionHandler?.IsMarginClick ?? false;

		private DisassemblyViewModel? _model;
		private CodeViewerSelectionHandler? _selectionHandler;
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

			InitContextMenu();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is DisassemblyViewModel model && _model != model) {
				_model = model;
				_model.SetViewer(_viewer);
				_selectionHandler = new CodeViewerSelectionHandler(_viewer, _model, (rowIndex, rowAddress) => rowAddress, true);
			}
			base.OnDataContextChanged(e);
		}

		private void InitContextMenu()
		{
			List<ContextMenuAction> actions = new List<ContextMenuAction> {
				MarkSelectionHelper.GetAction(
					() => CpuType.ToMemoryType(),
					() => Model.SelectionStart,
					() => Model.SelectionEnd,
					() => Model.Refresh(),
					() => !IsMarginClick
				),
				new ContextMenuAction() {
					ActionType = ActionType.EditSelectedCode,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditSelectedCode),
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => CpuType.SupportsAssembler() && EmuApi.IsPaused(),
					OnClick = () => {
						string code = Model.GetSelection(false, false, true, false, out int byteCount, true);
						AssemblerWindow.EditCode(CpuType, Model.SelectionStart, code, byteCount);
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Undo,
					IsEnabled = () => DebugApi.HasUndoHistory(),
					IsVisible = () => !IsMarginClick,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Undo),
					OnClick = () => {
						if(DebugApi.HasUndoHistory()) {
							DebugApi.PerformUndo();
							Model.Debugger.UpdateDisassembly(false);
						}
					}
				},
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.Copy,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Copy),
					IsVisible = () => !IsMarginClick,
					OnClick = () => Model.CopySelection()
				},
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.ToggleBreakpoint,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_ToggleBreakpoint),
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => !IsMarginClick,
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
					IsVisible = () => !IsMarginClick,
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
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.AbsAddress != null || (ActionLocation.RelAddress != null && ActionLocation.RelAddress.Value.Type.SupportsLabels()),
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						CodeLabel? label = loc.Label ?? (loc.AbsAddress.HasValue ? LabelManager.GetLabel(loc.AbsAddress.Value) : null);
						if(label != null) {
							LabelEditWindow.EditLabel(CpuType, this, label);
						} else if(loc.AbsAddress != null) {
							LabelEditWindow.EditLabel(CpuType, this, new CodeLabel(loc.AbsAddress.Value));
						} else if(loc.RelAddress != null) {
							LabelEditWindow.EditLabel(CpuType, this, new CodeLabel(loc.RelAddress.Value));
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditComment,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditComment),
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => false,
					AllowedWhenHidden = true,
					IsEnabled = () => ActionLocation.Label != null || ActionLocation.AbsAddress != null || (ActionLocation.RelAddress != null && ActionLocation.RelAddress.Value.Type.SupportsLabels()),
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						CodeLabel? label = loc.Label ?? (loc.AbsAddress.HasValue ? LabelManager.GetLabel(loc.AbsAddress.Value) : null);
						if(label != null) {
							CommentEditWindow.EditComment(this, label);
						} else if(loc.AbsAddress != null) {
							CommentEditWindow.EditComment(this, new CodeLabel(loc.AbsAddress.Value));
						}else if(loc.RelAddress != null) {
							CommentEditWindow.EditComment(this, new CodeLabel(loc.RelAddress.Value));
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_ViewInMemoryViewer),
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => !IsMarginClick,
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
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.FindOccurrences,
					HintText = () => GetSearchString() ?? "",
					IsVisible = () => !IsMarginClick,
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
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.MoveProgramCounter,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_MoveProgramCounter),
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => !IsMarginClick,
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
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => ActionLocation.RelAddress != null || ActionLocation.AbsAddress != null,
					OnClick = () => {
						Model.Debugger.RunToLocation(ActionLocation);
					}
				},
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_GoToLocation),
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => ActionLocation.RelAddress != null,
					OnClick = () => {
						LocationInfo loc = ActionLocation;
						if(loc.RelAddress != null) {
							Model.SetSelectedRow(loc.RelAddress.Value.Address, true, true);
						}
					}
				},
				new ContextMenuSeparator() { IsVisible = () => !IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.NavigateBack,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_NavigateBack),
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => Model.History.CanGoBack(),
					OnClick = () => {
						Model.GoBack();
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.NavigateForward,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_NavigateForward),
					IsVisible = () => !IsMarginClick,
					IsEnabled = () => Model.History.CanGoForward(),
					OnClick = () => {
						Model.GoForward();
					}
				},
			};

			actions.AddRange(GetBreakpointContextMenu());
			AddDisposables(DebugShortcutManager.CreateContextMenu(_viewer, actions));
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
				case CodeSegmentType.Token:
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

		private List<ContextMenuAction> GetBreakpointContextMenu()
		{
			Breakpoint? GetBreakpoint()
			{
				return ActionLocation.AbsAddress != null ? BreakpointManager.GetMatchingBreakpoint(ActionLocation.AbsAddress.Value, CpuType, true) : null;
			}

			return new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.SetBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint() == null && IsMarginClick,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuSeparator() { IsVisible = () => GetBreakpoint() == null && IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.CodeWindowEditBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible =() => IsMarginClick,
					IsEnabled = () => GetBreakpoint() != null,
					OnClick = () => {
						if(GetBreakpoint() is Breakpoint bp) {
							BreakpointEditWindow.EditBreakpoint(bp, this);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EnableBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint()?.Enabled == false && IsMarginClick,
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
					IsVisible = () => GetBreakpoint()?.Enabled != false && IsMarginClick,
					IsEnabled = () => GetBreakpoint()?.Enabled == true,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.EnableDisableBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuSeparator() { IsVisible = () => GetBreakpoint() != null && IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.RemoveBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => GetBreakpoint() != null && IsMarginClick,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
				new ContextMenuSeparator() { IsVisible = () => IsMarginClick },
				new ContextMenuAction() {
					ActionType = ActionType.ToggleForbidBreakpoint,
					HintText = () => GetHint(ActionLocation),
					IsVisible = () => IsMarginClick,
					OnClick = () => {
						if(ActionLocation.AbsAddress != null) {
							BreakpointManager.ToggleForbidBreakpoint(ActionLocation.AbsAddress.Value, CpuType);
						}
					}
				},
			};
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
			FocusViewer();
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
			if(e.Source is DisassemblyViewer) {
				PointerPointProperties props = e.GetCurrentPoint(this).Properties;
				if(_selectionHandler?.IsMarginClick == false && ActionLocation.RelAddress != null && props.IsLeftButtonPressed && e.ClickCount == 2) {
					Model.SetSelectedRow(ActionLocation.RelAddress.Value.Address, true, true);
				} else if(props.IsXButton1Pressed) {
					Model.GoBack();
				} else if(props.IsXButton2Pressed) {
					Model.GoForward();
				}
			}
		}

		private void FocusViewer()
		{
			Dispatcher.UIThread.Post(() => {
				//Focus disassembly view when selected by code
				if(_viewer.IsParentWindowFocused()) {
					_viewer.Focus();
				}
			});
		}

		private void Parent_Selected(object? sender, EventArgs e)
		{
			this.FocusViewer();
		}
	}
}
