using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Avalonia.Input;
using Avalonia.Interactivity;
using Mesen.Debugger.Utilities;

namespace Mesen.Debugger.Windows
{
	public class TilemapViewerWindow : Window, INotificationHandler
	{
		private TilemapViewerViewModel _model;
		private PictureViewer _picViewer;

		[Obsolete("For designer only")]
		public TilemapViewerWindow() : this(CpuType.Snes) { }

		public TilemapViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_picViewer = this.FindControl<PictureViewer>("picViewer");
			_model = new TilemapViewerViewModel(cpuType, _picViewer, this);
			DataContext = _model;

			_model.Config.LoadWindowSettings(this);

			if(Design.IsDesignMode) {
				return;
			}

			_picViewer.PointerMoved += PicViewer_PointerMoved;
			_picViewer.PointerLeave += PicViewer_PointerLeave;
			_picViewer.Source = _model.ViewerBitmap;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_model.RefreshData();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
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

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			ToolRefreshHelper.ProcessNotification(this, e, _model.Config.RefreshTiming, _model, _model.RefreshData);
		}
	}
}
