using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerViewModel : ViewModelBase
	{
		public SpriteViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		[Reactive] public SpritePreviewModel? SelectedSprite { get; set; }
		[Reactive] public DynamicTooltip? PreviewPanel { get; set; }
		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }
		[Reactive] public Rect SelectionRect { get; set; }
		[Reactive] public List<SpritePreviewModel> SpritePreviews { get; set; } = new();

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private Grid _spriteGrid;

		[Obsolete("For designer only")]
		public SpriteViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes, new PictureViewer(), new Grid(), null) { }

		public SpriteViewerViewModel(CpuType cpuType, ConsoleType consoleType, PictureViewer picViewer, Grid spriteGrid, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.SpriteViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;
			ConsoleType = consoleType;
			_spriteGrid = spriteGrid;

			InitBitmap(256, 256);

			FileMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			};

			ViewMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.EnableAutoRefresh,
					IsSelected = () => Config.RefreshTiming.AutoRefresh,
					OnClick = () => Config.RefreshTiming.AutoRefresh = !Config.RefreshTiming.AutoRefresh
				},
				new ContextMenuAction() {
					ActionType = ActionType.RefreshOnBreakPause,
					IsSelected = () => Config.RefreshTiming.RefreshOnBreakPause,
					OnClick = () => Config.RefreshTiming.RefreshOnBreakPause = !Config.RefreshTiming.RefreshOnBreakPause
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.ZoomIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomIn),
					OnClick = () => picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => picViewer.ZoomOut()
				},
			};

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			this.WhenAnyValue(x => x.SelectedSprite).Subscribe(x => UpdateSelectionPreview());
			
			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		public DynamicTooltip? GetPreviewPanel(SpritePreviewModel sprite, DynamicTooltip? existingTooltip)
		{
			TooltipEntries entries = existingTooltip?.Items ?? new();
			entries.AddPicture("Sprite", sprite.SpritePreview, (int)sprite.SpritePreviewZoom);
			entries.AddEntry("X, Y", sprite.X + ", " + sprite.Y);
			entries.AddEntry("Size", sprite.Width + "x" + sprite.Height);

			entries.AddEntry("Tile index", "$" + sprite.TileIndex.ToString("X2"));
			entries.AddEntry("Sprite index", sprite.SpriteIndex.ToString());
			entries.AddEntry("Visible", sprite.Visible);
			entries.AddEntry("Horizontal mirror", sprite.HorizontalMirror);
			entries.AddEntry("Vertical mirror", sprite.VerticalMirror);
			entries.AddEntry("Priority", sprite.Priority);

			if(existingTooltip != null) {
				return existingTooltip;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}

		public void UpdateSelectionPreview()
		{
			if(SelectedSprite != null) {
				PreviewPanel = GetPreviewPanel(SelectedSprite, PreviewPanel);
			} else {
				PreviewPanel = null;
			}
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap(int width, int height)
		{
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		private void InitGrid(int spriteCount)
		{
			double dpiScale = LayoutHelper.GetLayoutScale(_spriteGrid);
			if(_spriteGrid.Children.Count != spriteCount) {
				_spriteGrid.ColumnDefinitions.Clear();
				_spriteGrid.RowDefinitions.Clear();
				for(int i = 0; i < 8; i++) {
					_spriteGrid.ColumnDefinitions.Add(new() { Width = new GridLength(1, GridUnitType.Auto) });
				}

				for(int i = 0; i < spriteCount / 8; i++) {
					_spriteGrid.RowDefinitions.Add(new() { Height = new GridLength(1, GridUnitType.Auto) });
				}

				for(int i = 0; i < spriteCount; i++) {
					SpritePreviewPanel preview = new SpritePreviewPanel();
					preview.Height = 35.0 / dpiScale;
					preview.Width = 35.0 / dpiScale;
					Grid.SetColumn(preview, i % 8);
					Grid.SetRow(preview, i / 8);

					preview.DataContext = SpritePreviews[i];
					preview.PointerPressed += SpritePreview_PointerPressed;
					_spriteGrid.Children.Add(preview);
				}
			}
		}

		private void SpritePreview_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			if(sender is Control ctrl && ctrl.DataContext is SpritePreviewModel sprite) {
				SelectedSprite = sprite;
				UpdateSelection(sprite);
			}
		}

		private void InitPreviews(DebugSpriteInfo[] sprites)
		{
			if(SpritePreviews.Count != sprites.Length) {
				List<SpritePreviewModel> previews = new();
				for(int i = 0; i < sprites.Length; i++) {
					SpritePreviewModel model = new SpritePreviewModel();
					model.Init(ref sprites[i]);
					previews.Add(model);
				}
				SpritePreviews = previews;
			} else {
				for(int i = 0; i < sprites.Length; i++) {
					SpritePreviews[i].Init(ref sprites[i]);
				}
			}

			InitGrid(sprites.Length);
		}

		public void RefreshData()
		{
			switch(CpuType) {
				case CpuType.Cpu: RefreshData<PpuState>(); break;
				case CpuType.Nes: RefreshData<NesPpuState>(); break;
				case CpuType.Gameboy: RefreshData<GbPpuState>(); break;
			}
		}

		private void RefreshData<T>() where T : struct, BaseState
		{
			T ppuState = DebugApi.GetPpuState<T>(CpuType);
			byte[] vram = DebugApi.GetMemoryState(CpuType.GetVramMemoryType());
			byte[] spriteRam = DebugApi.GetMemoryState(CpuType.GetSpriteRamMemoryType());
			UInt32[] palette = PaletteHelper.GetConvertedPalette(CpuType, ConsoleType);

			Dispatcher.UIThread.Post(() => {
				GetSpritePreviewOptions options = new GetSpritePreviewOptions() {
					SelectedSprite = -1
				};
				UInt32[] palette = PaletteHelper.GetConvertedPalette(CpuType, ConsoleType);

				DebugSpritePreviewInfo size = DebugApi.GetSpritePreviewInfo(CpuType, options, ppuState);
				InitBitmap((int)size.Width, (int)size.Height);

				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetSpritePreview(CpuType, options, ppuState, vram, spriteRam, palette, framebuffer.FrameBuffer.Address);
				}

				DebugSpriteInfo[] sprites = DebugApi.GetSpriteList(CpuType, options, ppuState, vram, spriteRam, palette);
				InitPreviews(sprites);

				int selectedIndex = SelectedSprite?.SpriteIndex ?? -1;
				if(selectedIndex >= 0 && selectedIndex < SpritePreviews.Count) {
					SelectedSprite = SpritePreviews[selectedIndex];
					UpdateSelectionPreview();
				} else {
					SelectedSprite = null;
				}

				UpdateSelection(SelectedSprite);
			});
		}

		public void UpdateSelection(SpritePreviewModel? sprite)
		{
			if(sprite != null) {
				int offset = 0;
				if(CpuType == CpuType.Cpu) {
					offset = 256;
				}
				SelectionRect = new Rect(sprite.X + offset, sprite.Y, sprite.Width, sprite.Height);
			} else {
				SelectionRect = Rect.Empty;
			}
		}
	}

	public class SpritePreviewModel : ViewModelBase
	{
		[Reactive] public int SpriteIndex { get; set; }
		[Reactive] public int X { get; set; }
		[Reactive] public int Y { get; set; }
		[Reactive] public int Width { get; set; }
		[Reactive] public int Height { get; set; }
		[Reactive] public int TileIndex { get; set; }
		[Reactive] public int Priority { get; set; }
		[Reactive] public int Palette { get; set; }
		[Reactive] public bool Visible { get; set; }
		[Reactive] public string Size { get; set; } = "";
		
		[Reactive] public bool HorizontalMirror { get; set; }
		[Reactive] public bool VerticalMirror { get; set; }

		[Reactive] public DynamicBitmap SpritePreview { get; set; } = new DynamicBitmap(new PixelSize(1, 1), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
		[Reactive] public double SpritePreviewZoom { get; set; }

		public unsafe void Init(ref DebugSpriteInfo sprite)
		{
			SpriteIndex = sprite.SpriteIndex;
			X = sprite.X;
			Y = sprite.Y;
			Width = sprite.Width;
			Height = sprite.Height;
			TileIndex = sprite.TileIndex;
			Priority = sprite.Priority;
			Palette = sprite.Palette;
			
			Size = sprite.Width + "x" + sprite.Height;

			fixed(UInt32* p = sprite.SpritePreview) {
				if(SpritePreview.PixelSize.Width != sprite.Width || SpritePreview.PixelSize.Height != sprite.Height) {
					SpritePreview = new DynamicBitmap(new PixelSize(Width, Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
				}

				int spriteSize = Width * Height * sizeof(UInt32);
				using(var bitmapLock = SpritePreview.Lock()) {
					Buffer.MemoryCopy(p, (void*)bitmapLock.FrameBuffer.Address, spriteSize, spriteSize);
				}
			}
			
			SpritePreviewZoom = 32 / Math.Max(Width, Height);

			Visible = sprite.Visible;

			HorizontalMirror = sprite.HorizontalMirror;
			VerticalMirror = sprite.VerticalMirror;
			//flags += sprite.UseSecondTable ? "N" : "";
		}
	}
}
