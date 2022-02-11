using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Debugger.Controls;
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

		static DisassemblyView()
		{
			BoundsProperty.Changed.AddClassHandler<DisassemblyView>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				x.Model.VisibleRowCount = viewer.GetVisibleRowCount();
			});
		}

		public DisassemblyView()
		{
			InitializeComponent();
			Focusable = true;

			DebugShortcutManager.CreateContextMenu(this, new List<ContextMenuAction> {
				new ContextMenuAction() {
					ActionType = ActionType.MarkSelectionAs
				},
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
				},
				new ContextMenuAction() {
					ActionType = ActionType.AddWatch,
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditLabel,
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.GoToLocation,
				},
			});
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
				if(e.CodeLineData.Address < Model.SelectionStart || e.CodeLineData.Address > Model.SelectionEnd) {
					Model.SetSelectedRow(e.CodeLineData.Address);
				}
			}
		}

		public void Disassembly_CodePointerMoved(DisassemblyViewer sender, CodePointerMovedEventArgs e)
		{
			DynamicTooltip? tooltip;
			ICodeDataProvider? dp = Model.DataProvider;
			if(e.CodeSegment != null && dp != null && (tooltip = CodeTooltipHelper.GetTooltip(dp.CpuType, e.CodeSegment)) != null) {
				ToolTip.SetTip(this, tooltip);
				ToolTip.SetHorizontalOffset(this, 14);
				ToolTip.SetHorizontalOffset(this, 15);
				ToolTip.SetIsOpen(this, true);
			} else {
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
