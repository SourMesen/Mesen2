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

		private LocationInfo? _mouseOverCodeLocation;
		private LocationInfo? _contextMenuLocation;
		private ContextMenu _bpMarginContextMenu;
		private ContextMenu _mainContextMenu;
		private DisassemblyViewer _viewer;
		private bool _marginClicked;

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

		private LocationInfo ActionLocation
		{
			get
			{
				if(_viewer.ContextMenu == _bpMarginContextMenu) {
					return GetSelectedRowLocation();
				} else if(_viewer.ContextMenu?.IsOpen == true && _contextMenuLocation != null) {
					return _contextMenuLocation;
				} else if(_mouseOverCodeLocation != null) {
					return _mouseOverCodeLocation;
				}
				return GetSelectedRowLocation();
			}
		}

		private LocationInfo GetSelectedRowLocation()
		{
			CpuType cpuType = CpuType;
			AddressInfo relAddress = new AddressInfo() {
				Address = Model.SelectedRowAddress,
				Type = cpuType.ToMemoryType()
			};

			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
			return new LocationInfo() { RelAddress = relAddress, AbsAddress = absAddress };
		}

		public void Diassembly_RowClicked(DisassemblyViewer sender, RowClickedEventArgs e)
		{
			_marginClicked = e.MarginClicked;
			_viewer.ContextMenu = _mainContextMenu;

			if(e.Properties.IsLeftButtonPressed) {
				if(_marginClicked) {
					CpuType cpuType = e.CodeLineData.CpuType;
					AddressInfo relAddress = new AddressInfo() {
						Address = e.CodeLineData.Address,
						Type = cpuType.ToMemoryType()
					};
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					BreakpointManager.ToggleBreakpoint(absAddress.Address < 0 ? relAddress : absAddress, cpuType);
				} else {
					if(e.PointerEvent.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
						Model.ResizeSelectionTo(e.CodeLineData.Address);
					} else {
						Model.SetSelectedRow(e.CodeLineData.Address);
					}
				}
			} else if(e.Properties.IsRightButtonPressed) {
				if(_marginClicked) {
					_viewer.ContextMenu = _bpMarginContextMenu;
				}
				_contextMenuLocation = _mouseOverCodeLocation;

				if(e.CodeLineData.Address < Model.SelectionStart || e.CodeLineData.Address > Model.SelectionEnd) {
					Model.SetSelectedRow(e.CodeLineData.Address);
				}
			}
		}

		public void Disassembly_PointerLeave(object? sender, PointerEventArgs e)
		{
			_mouseOverCodeLocation = null;
		}

		public void Disassembly_CodePointerMoved(DisassemblyViewer sender, CodePointerMovedEventArgs e)
		{
			DynamicTooltip? tooltip = null;
			ICodeDataProvider dp = Model.DataProvider;

			if(e.CodeSegment != null) {
				_mouseOverCodeLocation = CodeTooltipHelper.GetLocation(dp.CpuType, e.CodeSegment);

				tooltip = CodeTooltipHelper.GetTooltip(dp.CpuType, e.CodeSegment);
				if(tooltip != null) {
					ToolTip.SetTip(this, tooltip);
					ToolTip.SetHorizontalOffset(this, 14);
					ToolTip.SetHorizontalOffset(this, 15);
					ToolTip.SetIsOpen(this, true);
				}
			} else {
				_mouseOverCodeLocation = null;
			}

			if(tooltip == null) {
				ToolTip.SetIsOpen(this, false);
				ToolTip.SetTip(this, null);
			}

			if(!_marginClicked && e.Data != null && e.PointerEvent.GetCurrentPoint(null).Properties.IsLeftButtonPressed) {
				Model.ResizeSelectionTo(e.Data.Address);
			}
		}

		public void Disassembly_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			Model.Scroll((int)(-e.Delta.Y * 3));
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.PageDown: Model.MoveCursor(Model.VisibleRowCount - 2, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.PageUp: Model.MoveCursor(-(Model.VisibleRowCount - 2), e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.Home: Model.ScrollToTop(); e.Handled = true; break;
				case Key.End: Model.ScrollToBottom(); e.Handled = true; break;

				case Key.Up: Model.MoveCursor(-1, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.Down: Model.MoveCursor(1, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
			}
		}
	}
}
