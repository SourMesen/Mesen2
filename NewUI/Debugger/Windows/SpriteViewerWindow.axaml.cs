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
using Avalonia.Media;

namespace Mesen.Debugger.Windows
{
	public class SpriteViewerWindow : Window
	{
		private NotificationListener _listener;
		private SpriteViewerViewModel _model;
		private PictureViewer _picViewer;
		private WriteableBitmap _viewerBitmap;
		private int _updateCounter = 0;

		public SpriteViewerWindow()
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
			if(this.DataContext is SpriteViewerViewModel model) {
				_model = model;
			} else {
				throw new Exception("Unexpected model");
			}
		}

		private void OnGridSelectionChanged(object? sender, SelectionChangedEventArgs e)
		{
			if(e.AddedItems[0] is SpriteViewerViewModel.SpriteRowModel row) {
				int offset = 0;
				if(_model.CpuType == CpuType.Cpu) {
					offset = 256;
				}
				_picViewer.SelectionRect = new Rect(row.X + offset, row.Y, row.Width, row.Height);
			}
		}

		private void UpdateSprites<T>(bool forceListUpdate) where T : struct, BaseState
		{
			T ppuState = DebugApi.GetPpuState<T>(_model.CpuType);
			byte[] vram = DebugApi.GetMemoryState(_model.CpuType.GetVramMemoryType());
			byte[] spriteRam = DebugApi.GetMemoryState(_model.CpuType.GetSpriteRamMemoryType());
			UInt32[] palette = PaletteHelper.GetConvertedPalette(_model.CpuType, _model.ConsoleType);

			Dispatcher.UIThread.Post(() => {
				GetSpritePreviewOptions options = new GetSpritePreviewOptions() {
					SelectedSprite = -1
				};
				UInt32[] palette = PaletteHelper.GetConvertedPalette(_model.CpuType, _model.ConsoleType);

				FrameInfo size = DebugApi.GetSpritePreviewSize(_model.CpuType, options, ppuState);
				if(_viewerBitmap.PixelSize.Width != size.Width || _viewerBitmap.PixelSize.Height != size.Height) {
					InitBitmap((int)size.Width, (int)size.Height);
				}

				using(var framebuffer = _viewerBitmap.Lock()) {
					DebugApi.GetSpritePreview(_model.CpuType, options, ppuState, vram, spriteRam, palette, framebuffer.Address);
				}

				_picViewer.Source = _viewerBitmap;
				_picViewer.InvalidateVisual();

				if(forceListUpdate || _updateCounter % 4 == 0) { //15 fps
					DebugSpriteInfo[] sprites = DebugApi.GetSpriteList(_model.CpuType, options, ppuState, spriteRam);
					_model.UpdateSprites(sprites);
				}
				_updateCounter++;
			});
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType == ConsoleNotificationType.PpuFrameDone || e.NotificationType == ConsoleNotificationType.CodeBreak) {
				bool forceListUpdate = e.NotificationType == ConsoleNotificationType.CodeBreak;
				switch(_model.CpuType) {
					case CpuType.Cpu: UpdateSprites<PpuState>(forceListUpdate); break;
					case CpuType.Nes: UpdateSprites<NesPpuState>(forceListUpdate); break;
					case CpuType.Gameboy: UpdateSprites<GbPpuState>(forceListUpdate); break;
				}
			}
		}
	}
}
