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
using System.Collections.Generic;

namespace Mesen.Debugger.Views
{
	public class DisassemblyView : UserControl
	{
		private DisassemblyViewModel Model => (DisassemblyViewModel)DataContext!;
		private LocationInfo? _mouseOverCodeLocation;
		private LocationInfo? _contextMenuLocation;

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
			Focusable = true;

			DebugShortcutManager.CreateContextMenu(this, new List<ContextMenuAction> {
				MarkSelectionHelper.GetAction(
					() => Model.DataProvider.CpuType.ToMemoryType(),
					() => Model.SelectionStart,
					() => Model.SelectionEnd,
					() => Model.Refresh()
				),
				new ContextMenuAction() {
					ActionType = ActionType.EditSelectedCode,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.CodeWindow_EditSelectedCode),
					IsEnabled = () => Model.DataProvider.CpuType.SupportsAssembler() && EmuApi.IsPaused(),
					OnClick = () => {
						string code = Model.GetSelection(false, false, true, false, out int byteCount);
						AssemblerWindow.EditCode(Model.DataProvider.CpuType, Model.SelectionStart, code, byteCount);
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
							BreakpointManager.ToggleBreakpoint(ActionLocation.AbsAddress.Value, Model.DataProvider.CpuType);
						} else if(ActionLocation.RelAddress != null) {
							BreakpointManager.ToggleBreakpoint(ActionLocation.RelAddress.Value, Model.DataProvider.CpuType);
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
							WatchManager.GetWatchManager(Model.DataProvider.CpuType).AddWatch("[" + ActionLocation.Label.Label + "]");
						} else if(ActionLocation.RelAddress != null) {
							WatchManager.GetWatchManager(Model.DataProvider.CpuType).AddWatch("[$" + ActionLocation.RelAddress.Value.Address.ToString(GetFormatString()) + "]");
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
							LabelEditWindow.EditLabel(this, label);
						} else if(ActionLocation.AbsAddress != null) {
							LabelEditWindow.EditLabel(this, new CodeLabel(ActionLocation.AbsAddress.Value));
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

		private string GetFormatString()
		{
			return Model.DataProvider.CpuType.ToMemoryType().GetFormatString();
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
				if(ContextMenu?.IsOpen == true && _contextMenuLocation != null) {
					return _contextMenuLocation;
				} else if(_mouseOverCodeLocation != null) {
					return _mouseOverCodeLocation;
				}
				return GetSelectedRowLocation();
			}
		}

		private LocationInfo GetSelectedRowLocation()
		{
			CpuType cpuType = Model.DataProvider.CpuType;
			AddressInfo relAddress = new AddressInfo() {
				Address = Model.SelectedRowAddress,
				Type = cpuType.ToMemoryType()
			};

			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
			return new LocationInfo() { RelAddress = relAddress, AbsAddress = absAddress };
		}

		public void Diassembly_RowClicked(DisassemblyViewer sender, RowClickedEventArgs e)
		{
			if(e.Properties.IsLeftButtonPressed) {
				if(e.MarginClicked) {
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
			}

			if(tooltip == null) {
				ToolTip.SetIsOpen(this, false);
				ToolTip.SetTip(this, null);
			}

			if(e.Data != null && e.PointerEvent.GetCurrentPoint(null).Properties.IsLeftButtonPressed) {
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
