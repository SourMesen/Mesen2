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
	public class SpriteViewerWindow : Window, INotificationHandler
	{
		private SpriteViewerViewModel _model;

		[Obsolete("For designer only")]
		public SpriteViewerWindow() : this(CpuType.Snes) { }

		public SpriteViewerWindow(CpuType cpuType)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			PictureViewer picViewer = this.FindControl<ScrollPictureViewer>("picViewer").InnerViewer;
			Grid spriteGrid = this.FindControl<Grid>("spriteGrid");
			_model = new SpriteViewerViewModel(cpuType, picViewer, spriteGrid, this);
			DataContext = _model;
		
			_model.Config.LoadWindowSettings(this);

			if(Design.IsDesignMode) {
				return;
			}

			picViewer.PointerMoved += PicViewer_PointerMoved;
			picViewer.PointerLeave += PicViewer_PointerLeave;
			picViewer.PositionClicked += PicViewer_PositionClicked;
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

				DynamicTooltip? tooltip = null;
				if(point != null) {
					SpritePreviewModel? sprite = _model.GetMatchingSprite(point.Value);
					tooltip = sprite == null ? null : _model.GetPreviewPanel(sprite, _model.ViewerTooltip);
				}

				if(tooltip != null) {
					_model.ViewerTooltip = tooltip;
					ToolTip.SetTip(viewer, tooltip);

					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 14);
					ToolTip.SetHorizontalOffset(viewer, 15);
					ToolTip.SetIsOpen(viewer, true);
				} else {
					ToolTip.SetTip(viewer, null);
					ToolTip.SetIsOpen(viewer, false);
					_model.ViewerTooltip = null;
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

		private void PicViewer_PositionClicked(object? sender, PositionClickedEventArgs e)
		{
			PixelPoint p = e.Position;
			SpritePreviewModel? sprite = _model.GetMatchingSprite(p);
			_model.SelectedSprite = sprite;
			_model.UpdateSelection(sprite);
			e.Handled = true;
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			if(e.NotificationType == ConsoleNotificationType.CodeBreak) {
				_model.ListView.ForceRefresh();
			}

			ToolRefreshHelper.ProcessNotification(this, e, _model.RefreshTiming, _model, _model.RefreshData);
		}
	}
}
