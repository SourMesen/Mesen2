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
using Mesen.Controls;
using System.Collections.Generic;
using Avalonia.Input;

namespace Mesen.Debugger.Windows
{
	public class SpriteViewerWindow : Window
	{
		private NotificationListener _listener;
		private SpriteViewerViewModel _model;
		private PictureViewer _picViewer;
		private WriteableBitmap _viewerBitmap;
		private Grid _spriteGrid;
		private List<SpritePreviewPanel> _previewPanels = new List<SpritePreviewPanel>();

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

			_spriteGrid = this.FindControl<Grid>("spriteGrid");
		}

		private void InitGrid(DebugSpriteInfo[] sprites)
		{
			if(_previewPanels.Count != sprites.Length) {
				_spriteGrid.ColumnDefinitions.Clear();
				_spriteGrid.RowDefinitions.Clear();
				for(int i = 0; i < 8; i++) {
					_spriteGrid.ColumnDefinitions.Add(new() { Width = new GridLength(1, GridUnitType.Auto) });
				}

				for(int i = 0; i < sprites.Length / 8; i++) {
					_spriteGrid.RowDefinitions.Add(new() { Height = new GridLength(1, GridUnitType.Auto) });
				}

				_previewPanels.Clear();
				for(int i = 0; i < sprites.Length; i++) {
					SpritePreviewPanel preview = new SpritePreviewPanel();
					preview.Height = 35;
					preview.Width = 35;
					Grid.SetColumn(preview, i % 8);
					Grid.SetRow(preview, i / 8);

					SpritePreviewModel model = new SpritePreviewModel();
					model.Init(ref sprites[i]);
					preview.DataContext = model;

					_spriteGrid.Children.Add(preview);
					_previewPanels.Add(preview);
					preview.PointerPressed += SpritePreview_PointerPressed;
				}
			} else {
				for(int i = 0; i < sprites.Length; i++) {
					((SpritePreviewModel)_previewPanels[i].DataContext!).Init(ref sprites[i]);
				}
			}
		}

		private void SpritePreview_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			if(sender is Control ctrl && ctrl.DataContext is SpritePreviewModel sprite) {
				_model.SelectedSprite = sprite;
				UpdateSelection(sprite);
			}
		}

		private void ScreenPreview_PositionClicked(object? sender, PositionClickedEventArgs e)
		{
			Point p = e.Position;
			if(_model.CpuType == CpuType.Cpu) {
				p = p.WithX(p.X - 256);
			}

			foreach(SpritePreviewPanel previewPanel in _previewPanels) {
				if(previewPanel.DataContext is SpritePreviewModel sprite) {
					if(
						p.X >= sprite.X && p.X < sprite.X + sprite.Width &&
						p.Y >= sprite.Y && p.Y < sprite.Y + sprite.Height
					) {
						_model.SelectedSprite = sprite;
						UpdateSelection(sprite);
						break;
					}
				}
			}
			e.Handled = true;
		}

		private void UpdateSelection(SpritePreviewModel? sprite)
		{
			if(sprite != null) {
				int offset = 0;
				if(_model.CpuType == CpuType.Cpu) {
					offset = 256;
				}
				_picViewer.SelectionRect = new Rect(sprite.X + offset, sprite.Y, sprite.Width, sprite.Height);
			} else {
				_picViewer.SelectionRect = Rect.Empty;
			}
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

				DebugSpritePreviewInfo size = DebugApi.GetSpritePreviewInfo(_model.CpuType, options, ppuState);
				if(_viewerBitmap.PixelSize.Width != size.Width || _viewerBitmap.PixelSize.Height != size.Height) {
					InitBitmap((int)size.Width, (int)size.Height);
				}

				using(var framebuffer = _viewerBitmap.Lock()) {
					DebugApi.GetSpritePreview(_model.CpuType, options, ppuState, vram, spriteRam, palette, framebuffer.Address);
				}

				_picViewer.Source = _viewerBitmap;
				_picViewer.InvalidateVisual();

				DebugSpriteInfo[] sprites = DebugApi.GetSpriteList(_model.CpuType, options, ppuState, vram, spriteRam, palette);
				InitGrid(sprites);

				int selectedIndex = _model.SelectedSprite?.SpriteIndex ?? -1;
				if(selectedIndex >= 0 && selectedIndex < _previewPanels.Count) {
					_model.SelectedSprite = ((SpritePreviewModel)_previewPanels[selectedIndex].DataContext!);
				} else {
					_model.SelectedSprite = null;
				}
				UpdateSelection(_model.SelectedSprite);
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
