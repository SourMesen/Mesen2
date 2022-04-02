using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Debugger.Controls;

namespace Mesen.Debugger.ViewModels
{
	public interface IMouseOverViewerModel
	{
		DynamicTooltip? ViewerTooltip { get; set; }
		PixelPoint? ViewerMousePos { get; set; }

		DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate);
	}

	public class MouseViewerModelEvents
	{
		private IMouseOverViewerModel _model;
		private Window _wnd;

		public static void InitEvents(IMouseOverViewerModel model, Window wnd, PictureViewer viewer)
		{
			new MouseViewerModelEvents(model, wnd, viewer);
		}

		private MouseViewerModelEvents(IMouseOverViewerModel model, Window wnd, PictureViewer viewer)
		{
			_model = model;
			_wnd = wnd;
			viewer.PointerMoved += PicViewer_PointerMoved;
			viewer.PointerLeave += PicViewer_PointerLeave;
		}

		private void PicViewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				if(_wnd.IsActive) {
					viewer.Focus();
				}

				PixelPoint? point = viewer.GetGridPointFromMousePoint(e.GetCurrentPoint(viewer).Position);
				if(point == _model.ViewerMousePos) {
					return;
				}
				_model.ViewerMousePos = point;

				_model.ViewerTooltip = point == null ? null : _model.GetPreviewPanel(point.Value, _model.ViewerTooltip);

				if(_model.ViewerTooltip != null) {
					ToolTip.SetTip(viewer, _model.ViewerTooltip);

					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 14);
					ToolTip.SetHorizontalOffset(viewer, 15);
					ToolTip.SetIsOpen(viewer, true);
				} else {
					_model.ViewerTooltip = null;
					ToolTip.SetTip(viewer, null);
					ToolTip.SetIsOpen(viewer, false);
				}
			}
		}

		private void PicViewer_PointerLeave(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				ToolTip.SetTip(viewer, null);
				ToolTip.SetIsOpen(viewer, false);
			}
			_model.ViewerTooltip = null;
			_model.ViewerMousePos = null;
		}
	}
}