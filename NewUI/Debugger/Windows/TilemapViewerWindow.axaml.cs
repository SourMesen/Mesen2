#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Avalonia.Platform;
using Mesen.Interop;
using System.ComponentModel;
using Avalonia.Input;
using Avalonia.Interactivity;

namespace Mesen.Debugger.Windows
{
	public class TilemapViewerWindow : Window
	{
		private NotificationListener _listener;
		private TilemapViewerViewModel _model;
		private PictureViewer _picViewer;
		
		public TilemapViewerWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			//Renderer.DrawFps = true;
			_picViewer = this.FindControl<PictureViewer>("picViewer");
			_picViewer.PointerMoved += PicViewer_PointerMoved;
			_picViewer.PointerLeave += PicViewer_PointerLeave;
			_picViewer.Source = _model.ViewerBitmap;

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_listener?.Dispose();
			base.OnClosing(e);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is TilemapViewerViewModel model) {
				_model = model;
			} else {
				throw new Exception("Unexpected model");
			}
		}

		private void PicViewer_PointerMoved(object? sender, PointerEventArgs e)
		{
			if(sender is PictureViewer viewer) {
				PixelPoint p = PixelPoint.FromPoint(e.GetCurrentPoint(viewer).Position / viewer.Zoom, 1);
				DynamicTooltip? tooltip = _model.GetPreviewPanel(p);

				if(tooltip != null) {
					ToolTip.SetTip(viewer, tooltip);

					//Force tooltip to update its position
					ToolTip.SetHorizontalOffset(viewer, 4);
					ToolTip.SetHorizontalOffset(viewer, 5);
					ToolTip.SetIsOpen(viewer, true);
				} else {
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
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowSettingsPanel = !_model.Config.ShowSettingsPanel;
		}

		private void UpdateTilemap<T>() where T : struct, BaseState
		{
			T ppuState = DebugApi.GetPpuState<T>(_model.CpuType);
			byte[] vram = DebugApi.GetMemoryState(_model.CpuType.GetVramMemoryType());
			byte[]? prevVram = _model.PrevVram;
			_model.PpuState = ppuState;
			_model.PrevVram = vram;
			UInt32[] palette = PaletteHelper.GetConvertedPalette(_model.CpuType, _model.ConsoleType);

			Dispatcher.UIThread.Post(() => {
				_model.UpdateBitmap(ppuState, vram, prevVram);

				_picViewer.Source = _model.ViewerBitmap;
				_picViewer.InvalidateVisual();
			});
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}

			switch(_model.CpuType) {
				case CpuType.Cpu: UpdateTilemap<PpuState>(); break;
				case CpuType.Nes: UpdateTilemap<NesPpuState>(); break;
				case CpuType.Gameboy: UpdateTilemap<GbPpuState>(); break;
			}
		}
	}
}
