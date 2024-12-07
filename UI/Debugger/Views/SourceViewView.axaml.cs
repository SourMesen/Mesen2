using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
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
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.Views
{
	public class SourceViewView : MesenUserControl
	{
		private SourceViewViewModel Model => (SourceViewViewModel)DataContext!;
		private LocationInfo ActionLocation => _selectionHandler?.ActionLocation ?? new LocationInfo();
		private bool IsMarginClick => _selectionHandler?.IsMarginClick ?? false;
		private CpuType CpuType => Model.CpuType;

		private SourceViewViewModel? _model;
		private CodeViewerSelectionHandler? _selectionHandler;
		private DisassemblyViewer _viewer;
		private BaseToolContainerViewModel? _parentModel;

		public SourceViewView()
		{
			InitializeComponent();
			_viewer = this.GetControl<DisassemblyViewer>("disViewer");
			_viewer.GetPropertyChangedObservable(DisassemblyViewer.VisibleRowCountProperty).Subscribe(x => {
				SourceViewViewModel? model = Model;
				if(model == null) {
					return;
				}

				int rowCount = _viewer.VisibleRowCount;
				int prevCount = model.VisibleRowCount;
				if(prevCount != rowCount) {
					int pos = model.ScrollPosition + (prevCount / 2);
					model.VisibleRowCount = rowCount;
					model.ScrollToRowNumber(pos);
					model.Refresh();
				}
			});

			InitContextMenu();
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is SourceViewViewModel model && _model != model) {
				if(_model != null) {
					model.VisibleRowCount = _model.VisibleRowCount;
				}
				_model = model;
				_model.SetViewer(_viewer);
				_selectionHandler?.Dispose();
				if(model != null) {
					_selectionHandler = new CodeViewerSelectionHandler(_viewer, _model, (rowIndex, rowAddress) => rowIndex + _model.ScrollPosition, true);
				}
			}
			base.OnDataContextChanged(e);
		}

		private void InitContextMenu()
		{
			List<ContextMenuAction> actions = new List<ContextMenuAction> {
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
					IsEnabled = () => ActionLocation.Symbol != null || ActionLocation.RelAddress != null,
					OnClick = () => GoToLocation(ActionLocation)
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

		private void GoToLocation(LocationInfo loc)
		{
			if(loc.Symbol != null) {
				AddressInfo? addr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(loc.Symbol.Value);
				if(addr?.Address > 0) {
					SourceCodeLocation? srcLoc = DebugWorkspaceManager.SymbolProvider?.GetSourceCodeLineInfo(addr.Value);
					if(srcLoc != null) {
						_model?.ScrollToLocation(srcLoc.Value, true);
					}
				}
			} else if(loc.RelAddress != null) {
				_model?.GoToRelativeAddress(loc.RelAddress.Value.Address, true);
			}
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
				return ActionLocation.AbsAddress != null ? BreakpointManager.GetMatchingBreakpoint(ActionLocation.AbsAddress.Value, CpuType) : null;
			}

			return new List<ContextMenuAction> {
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
					IsVisible = () => IsMarginClick,
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
				}
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

			if(loc?.Symbol != null) {
				return loc.Symbol.Value.Name + (loc.LabelAddressOffset > 0 ? ("+" + loc.LabelAddressOffset) : "");
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
			if(_model?.ActiveAddress != null) {
				//Go to active address when clicking on the source view tab
				_model?.GoToRelativeAddress(_model.ActiveAddress.Value, true);
			}
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
				if(_selectionHandler?.IsMarginClick == false && props.IsLeftButtonPressed && e.ClickCount == 2) {
					GoToLocation(ActionLocation);
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
				//Focus source view when selected by code
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
