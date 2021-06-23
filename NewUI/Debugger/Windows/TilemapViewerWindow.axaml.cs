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

namespace Mesen.Debugger.Windows
{
	public class TilemapViewerWindow : Window
	{
		private NotificationListener _listener;
		private TilemapViewerViewModel _model;
		private PictureViewer _picViewer;
		private WriteableBitmap _viewerBitmap;
		private byte[]? _prevVram = null;
		private TilemapViewerTab _selectedTab = new TilemapViewerTab();

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
			_picViewer.Source = _viewerBitmap;
			InitBitmap(256, 256);

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}
		
		protected override void OnClosing(CancelEventArgs e)
		{
			_listener?.Dispose();
			base.OnClosing(e);
		}

		private void InitBitmap(int width, int height)
		{
			_viewerBitmap = new WriteableBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is TilemapViewerViewModel model) {
				_model = model;
			} else {
				throw new Exception("Unexpected model");
			}
		}

		private void OnTabSelected(object? sender, SelectionChangedEventArgs e)
		{
			if(e.AddedItems.Count > 0) {
				_selectedTab = (TilemapViewerTab)e.AddedItems[0]!;
			}
		}

		private void UpdateTilemap<T>() where T : struct, BaseState
		{
			T ppuState = DebugApi.GetPpuState<T>(_model.CpuType);
			byte[] vram = DebugApi.GetMemoryState(_model.CpuType.GetVramMemoryType());
			byte[]? prevVram = _prevVram;
			_prevVram = vram;
			UInt32[] palette = PaletteHelper.GetConvertedPalette(_model.CpuType, _model.ConsoleType);

			Dispatcher.UIThread.Post(() => {
				GetTilemapOptions options = new GetTilemapOptions() {
					Layer = (byte)_selectedTab.Layer,
					CompareVram = prevVram,
					HighlightTileChanges = _model.HighlightTileChanges,
					HighlightAttributeChanges = _model.HighlightAttributeChanges,
					DisplayMode = _model.DisplayMode
				};
				UInt32[] palette = PaletteHelper.GetConvertedPalette(_model.CpuType, _model.ConsoleType);

				FrameInfo size = DebugApi.GetTilemapSize(_model.CpuType, options, ppuState);
				if(_viewerBitmap.PixelSize.Width != size.Width || _viewerBitmap.PixelSize.Height != size.Height) {
					InitBitmap((int)size.Width, (int)size.Height);
				}

				using(var framebuffer = _viewerBitmap.Lock()) {
					DebugApi.GetTilemap(_model.CpuType, options, ppuState, vram, palette, framebuffer.Address);
				}

				_picViewer.Source = _viewerBitmap;
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
