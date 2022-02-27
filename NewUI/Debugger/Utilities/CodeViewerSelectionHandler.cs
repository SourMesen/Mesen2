using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Mesen.Debugger.Utilities;
using Mesen.Interop;

namespace Mesen.Debugger.Utilities
{
	public class CodeViewerSelectionHandler
	{
		private DisassemblyViewer _viewer;
		private ISelectableModel _model;
		private LocationInfo? _mouseOverCodeLocation;
		private LocationInfo? _contextMenuLocation;
		
		private bool _allowMarginClick = false;
		private bool _marginClicked = false;
		private bool _useRowIndex = false;

		private ContextMenu? _mainContextMenu;
		private ContextMenu? _marginContextMenu;

		public CodeViewerSelectionHandler(DisassemblyViewer viewer, ISelectableModel model, bool useRowIndex, ContextMenu? mainContextMenu = null, ContextMenu? marginContextMenu = null)
		{
			_viewer = viewer;
			_model = model;
			_useRowIndex = useRowIndex;

			_mainContextMenu = mainContextMenu;
			_marginContextMenu = marginContextMenu;
			_allowMarginClick = _marginContextMenu != null;

			_viewer.PointerLeave += Viewer_PointerLeave;
			_viewer.RowClicked += Viewer_RowClicked;
			_viewer.CodePointerMoved += Viewer_CodePointerMoved;
			_viewer.PointerWheelChanged += Viewer_PointerWheelChanged;
			_viewer.KeyDown += Viewer_KeyDown;
		}

		private void Viewer_PointerLeave(object? sender, PointerEventArgs e)
		{
			_mouseOverCodeLocation = null;
		}

		public int GetAddress(RowClickedEventArgs e)
		{
			return _useRowIndex ? e.RowNumber : e.CodeLineData.Address;
		}

		public int GetAddress(CodePointerMovedEventArgs e)
		{
			return _useRowIndex ? e.RowNumber : (e.Data?.Address ?? -1);
		}

		public void Viewer_RowClicked(DisassemblyViewer sender, RowClickedEventArgs e)
		{
			_marginClicked = e.MarginClicked && _allowMarginClick;
			_viewer.ContextMenu = _mainContextMenu;

			if(e.Properties.IsLeftButtonPressed) {
				if(_marginClicked) {
					CpuType cpuType = e.CodeLineData.CpuType;
					AddressInfo relAddress = new AddressInfo() {
						Address = GetAddress(e),
						Type = cpuType.ToMemoryType()
					};
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					BreakpointManager.ToggleBreakpoint(absAddress.Address < 0 ? relAddress : absAddress, cpuType);
				} else {
					if(e.PointerEvent.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
						_model.ResizeSelectionTo(GetAddress(e));
					} else {
						_model.SetSelectedRow(GetAddress(e));
					}
				}
			} else if(e.Properties.IsRightButtonPressed) {
				if(_marginClicked) {
					_viewer.ContextMenu = _marginContextMenu;
				}
				_contextMenuLocation = _mouseOverCodeLocation;

				if(!_model.IsSelected(GetAddress(e))) {
					_model.SetSelectedRow(GetAddress(e));
				}
			}
		}

		public void Viewer_CodePointerMoved(DisassemblyViewer sender, CodePointerMovedEventArgs e)
		{
			DynamicTooltip? tooltip = null;

			if(e.CodeSegment != null && e.Data != null) {
				_mouseOverCodeLocation = CodeTooltipHelper.GetLocation(e.Data.CpuType, e.CodeSegment);

				tooltip = CodeTooltipHelper.GetTooltip(e.Data.CpuType, e.CodeSegment);
				if(tooltip != null) {
					ToolTip.SetTip(_viewer, tooltip);
					ToolTip.SetHorizontalOffset(_viewer, 14);
					ToolTip.SetHorizontalOffset(_viewer, 15);
					ToolTip.SetIsOpen(_viewer, true);
				}
			} else {
				_mouseOverCodeLocation = null;
			}

			if(tooltip == null) {
				ToolTip.SetIsOpen(_viewer, false);
				ToolTip.SetTip(_viewer, null);
			}

			if(!_marginClicked && GetAddress(e) >= 0 && e.PointerEvent.GetCurrentPoint(null).Properties.IsLeftButtonPressed) {
				_model.ResizeSelectionTo(GetAddress(e));
			}
		}

		public void Viewer_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			_model.Scroll((int)(-e.Delta.Y * 3));
		}

		private void Viewer_KeyDown(object? sender, KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.PageDown: _model.MoveCursor(_model.VisibleRowCount - 2, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.PageUp: _model.MoveCursor(-(_model.VisibleRowCount - 2), e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.Home: _model.ScrollToTop(); e.Handled = true; break;
				case Key.End: _model.ScrollToBottom(); e.Handled = true; break;

				case Key.Up: _model.MoveCursor(-1, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
				case Key.Down: _model.MoveCursor(1, e.KeyModifiers.HasFlag(KeyModifiers.Shift)); e.Handled = true; break;
			}
		}

		public LocationInfo ActionLocation
		{
			get
			{
				if(_viewer.ContextMenu == _marginContextMenu) {
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
			AddressInfo? relAddress = _model.GetSelectedRowAddress();
			AddressInfo? absAddress = null;
			
			if(relAddress != null) {
				absAddress = DebugApi.GetAbsoluteAddress(relAddress.Value);
			}
			
			return new LocationInfo() { RelAddress = relAddress, AbsAddress = absAddress };
		}
	}
}
