using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Debugger.Controls;
using Mesen.Utilities;

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
			viewer.PointerExited += PicViewer_PointerExited;
		}

		private void PicViewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PixelPoint? point = viewer.GetGridPointFromMousePoint(e.GetCurrentPoint(viewer).Position);
				if(point == _model.ViewerMousePos) {
					return;
				}
				_model.ViewerMousePos = point;

				_model.ViewerTooltip = point == null ? null : _model.GetPreviewPanel(point.Value, _model.ViewerTooltip);

				if(_model.ViewerTooltip != null) {
					TooltipHelper.ShowTooltip(viewer, _model.ViewerTooltip, 15);
				} else {
					_model.ViewerTooltip = null;
					TooltipHelper.HideTooltip(viewer);
				}
			}
		}

		private void PicViewer_PointerExited(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				TooltipHelper.HideTooltip(viewer);
			}
			_model.ViewerTooltip = null;
			_model.ViewerMousePos = null;
		}
	}
}