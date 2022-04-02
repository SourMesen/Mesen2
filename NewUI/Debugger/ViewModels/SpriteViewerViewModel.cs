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
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerViewModel : DisposableViewModel, ICpuTypeModel, IMouseOverViewerModel
	{
		public SpriteViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }
		
		[Reactive] public CpuType CpuType { get; set; }

		[Reactive] public SpritePreviewModel? SelectedSprite { get; set; }
		[Reactive] public DynamicTooltip? SelectedPreviewPanel { get; set; }
		
		[Reactive] public DynamicTooltip? PreviewPanelTooltip { get; set; }
		[Reactive] public SpritePreviewModel? PreviewPanelSprite { get; set; }

		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }
		[Reactive] public Rect SelectionRect { get; set; }
		[Reactive] public Rect? MouseOverRect { get; set; }

		[Reactive] public List<SpritePreviewModel> SpritePreviews { get; set; } = new();
		[Reactive] public List<Rect>? SpriteRects { get; set; } = null;

		public SpriteViewerListViewModel ListView { get; }
		
		[Reactive] public int MaxSourceOffset { get; set; } = 0;

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private Grid _spriteGrid;
		private BaseState? _ppuState;
		private byte[] _spriteRam = Array.Empty<byte>();
		private byte[] _vram = Array.Empty<byte>();
		private RefStruct<DebugPaletteInfo>? _palette = null;

		[Obsolete("For designer only")]
		public SpriteViewerViewModel() : this(CpuType.Snes, new PictureViewer(), new Grid(), new Control(), null) { }

		public SpriteViewerViewModel(CpuType cpuType, PictureViewer picViewer, Grid spriteGrid, Control listView, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.SpriteViewer;

			ListView = AddDisposable(new SpriteViewerListViewModel(this));

			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;
			_spriteGrid = spriteGrid;

			InitBitmap(256, 256);

			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd?.Close()
				}
			});

			ViewMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.Refresh,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Refresh),
					OnClick = () => RefreshData()
				},
				new ContextMenuSeparator(),
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
				new ContextMenuSeparator(),
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
			});

			DebugShortcutManager.CreateContextMenu(picViewer, new List<object> {
				GetViewInMemoryViewerAction(),
				GetViewInTileViewerAction(),
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					OnClick = () => picViewer.ExportToPng()
				}
			});

			DebugShortcutManager.CreateContextMenu(_spriteGrid, new List<object> {
				GetViewInMemoryViewerAction(),
				GetViewInTileViewerAction()
			});

			DebugShortcutManager.CreateContextMenu(listView, new List<object> {
				GetViewInMemoryViewerAction(),
				GetViewInTileViewerAction()
			});

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			AddDisposable(this.WhenAnyValue(x => x.CpuType).Subscribe(_ => {
				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.SelectedSprite).Subscribe(x => {
				UpdateSelectionPreview();
				if(x != null) {
					ListView.SelectSprite(x.SpriteIndex);
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.ViewerMousePos, x => x.PreviewPanelSprite).Subscribe(x => UpdateMouseOverRect()));

			AddDisposable(this.WhenAnyValue(x => x.Config.Source, x => x.Config.SourceOffset).Subscribe(x => RefreshData()));

			ListView.InitListViewObservers();

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, Config_PropertyChanged));

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		private ContextMenuAction GetViewInMemoryViewerAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.ViewInMemoryViewer,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_ViewInMemoryViewer),
				IsEnabled = () => GetSelectedSprite() != null,
				OnClick = () => {
					SpritePreviewModel? sprite = GetSelectedSprite();
					if(sprite?.TileAddress >= 0) {
						MemoryToolsWindow.ShowInMemoryTools(CpuType.GetVramMemoryType(), sprite.TileAddress);
					}
				}
			};
		}

		private ContextMenuAction GetViewInTileViewerAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.ViewInTileViewer,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_ViewInTileViewer),
				OnClick = () => {
					SpritePreviewModel? sprite = GetSelectedSprite();
					if(sprite?.TileAddress >= 0 && _palette != null) {
						DebugPaletteInfo pal = _palette.Get();
						int paletteOffset = (int)(pal.BgColorCount / pal.ColorsPerPalette);
						TileViewerWindow.OpenAtTile(CpuType, CpuType.GetVramMemoryType(), sprite.TileAddress, sprite.Format, TileLayout.Normal, sprite.Palette + paletteOffset);
					}
				}
			};
		}

		private SpritePreviewModel? GetSelectedSprite()
		{
			return PreviewPanelSprite ?? SelectedSprite;
		}

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			RefreshTab();
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			SpritePreviewModel? sprite = GetMatchingSprite(p);
			return sprite == null ? null : GetPreviewPanel(sprite, tooltipToUpdate);
		}

		public DynamicTooltip? GetPreviewPanel(SpritePreviewModel sprite, DynamicTooltip? existingTooltip)
		{
			if(_palette == null) {
				return null;
			}

			TooltipEntries entries = existingTooltip?.Items ?? new();
			entries.StartUpdate();
			if(sprite.SpritePreview != null) {
				entries.AddPicture("Sprite", sprite.SpritePreview, 48.0 / sprite.Width);
			}

			DebugPaletteInfo palette = _palette.Get();
			int paletteSize = (int)Math.Pow(2, sprite.Bpp);
			int paletteIndex = sprite.Palette >= 0 ? sprite.Palette : 0;
			UInt32[] spritePalette = new UInt32[paletteSize];
			Array.Copy(palette.GetRgbPalette(), palette.BgColorCount + paletteIndex * paletteSize, spritePalette, 0, paletteSize);

			entries.AddEntry("Palette", spritePalette);

			entries.AddEntry("Sprite index", sprite.SpriteIndex.ToString());
			entries.AddEntry("X, Y", 
				"$" + sprite.X.ToString("X2") + ", $" + sprite.Y.ToString("X2") + Environment.NewLine +
				sprite.X + ", " + sprite.Y
			);
			entries.AddEntry("Size", sprite.Width + "x" + sprite.Height);

			entries.AddEntry("Tile index", "$" + sprite.TileIndex.ToString("X2"));
			entries.AddEntry("Tile address", "$" + sprite.TileAddress.ToString("X4"));
			entries.AddEntry("Palette index", sprite.Palette.ToString());
			entries.AddEntry("Palette address", "$" + sprite.PaletteAddress.ToString("X2"));
			entries.AddEntry("Visible", sprite.Visible);
			entries.AddEntry("Horizontal mirror", sprite.HorizontalMirror);
			entries.AddEntry("Vertical mirror", sprite.VerticalMirror);
			entries.AddEntry("Priority", ResourceHelper.GetEnumText(sprite.Priority));
			if(sprite.UseSecondTable != NullableBoolean.Undefined) {
				entries.AddEntry("Second table", sprite.UseSecondTable == NullableBoolean.True);
			}
			entries.EndUpdate();

			if(existingTooltip != null) {
				return existingTooltip;
			} else {
				return new DynamicTooltip() { Items = entries };
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
				_spriteGrid.Children.Clear();
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
					preview.Height = 2 + 32.0 / dpiScale;
					preview.Width = 2 + 32.0 / dpiScale;
					Grid.SetColumn(preview, i % 8);
					Grid.SetRow(preview, i / 8);

					preview.DataContext = SpritePreviews[i];
					preview.PointerPressed += SpritePreview_PointerPressed;
					preview.PointerEnter += SpritePreview_PointerEnter;
					preview.PointerLeave += SpritePreview_PointerLeave;
					_spriteGrid.Children.Add(preview);
				}
			}
		}

		private void SpritePreview_PointerLeave(object? sender, PointerEventArgs e)
		{
			if(sender is SpritePreviewPanel ctrl) {
				ToolTip.SetTip(ctrl, null);
				ToolTip.SetIsOpen(ctrl, false);
			}
			PreviewPanelTooltip = null;
			PreviewPanelSprite = null;
		}

		private void SpritePreview_PointerEnter(object? sender, PointerEventArgs e)
		{
			if(sender is Control ctrl && ctrl.DataContext is SpritePreviewModel sprite) {
				PreviewPanelTooltip = GetPreviewPanel(sprite, PreviewPanelTooltip);
				PreviewPanelSprite = sprite;

				ToolTip.SetTip(ctrl, PreviewPanelTooltip);
				ToolTip.SetHorizontalOffset(ctrl, 15);
				ToolTip.SetIsOpen(ctrl, true);
			}
		}

		private void SpritePreview_PointerPressed(object? sender, PointerPressedEventArgs e)
		{
			if(sender is Control ctrl && ctrl.DataContext is SpritePreviewModel sprite) {
				SelectedSprite = sprite;
				UpdateSelection(sprite);
			}
		}

		public void SelectSprite(int spriteIndex)
		{
			SelectedSprite = SpritePreviews[spriteIndex];
			UpdateSelection(SelectedSprite);
		}

		private void InitPreviews(DebugSpriteInfo[] sprites, DebugSpritePreviewInfo previewInfo)
		{
			if(SpritePreviews.Count != sprites.Length) {
				List<SpritePreviewModel> previews = new();
				for(int i = 0; i < sprites.Length; i++) {
					SpritePreviewModel model = new SpritePreviewModel();
					model.Init(ref sprites[i], previewInfo);
					previews.Add(model);
				}
				SpritePreviews = previews;
			} else {
				for(int i = 0; i < sprites.Length; i++) {
					SpritePreviews[i].Init(ref sprites[i], previewInfo);
				}
			}

			InitGrid(sprites.Length);
		}

		public void RefreshData()
		{
			switch(CpuType) {
				case CpuType.Snes: RefreshData<SnesPpuState>(); break;
				case CpuType.Nes: RefreshData<NesPpuState>(); break;
				case CpuType.Gameboy: RefreshData<GbPpuState>(); break;
			}
		}

		private void RefreshData<T>() where T : struct, BaseState
		{
			_ppuState = DebugApi.GetPpuState<T>(CpuType);
			_vram = DebugApi.GetMemoryState(CpuType.GetVramMemoryType());

			MemoryType spriteMemType = CpuType.GetSpriteRamMemoryType();
			int spriteRamSize = DebugApi.GetMemorySize(spriteMemType);

			if(Config.Source == SpriteViewerSource.SpriteRam) {
				_spriteRam = DebugApi.GetMemoryState(spriteMemType);
			} else {
				MemoryType cpuMemory = CpuType.ToMemoryType();
				_spriteRam = DebugApi.GetMemoryValues(cpuMemory, (uint)Config.SourceOffset, (uint)(Config.SourceOffset + spriteRamSize - 1));
				MaxSourceOffset = DebugApi.GetMemorySize(cpuMemory) - spriteRamSize;
			}

			_palette = new RefStruct<DebugPaletteInfo>(DebugApi.GetPaletteInfo(CpuType));

			RefreshTab();
		}

		private void RefreshTab()
		{
			Dispatcher.UIThread.Post(() => {
				if(_ppuState == null || _palette == null) {
					return;
				}

				GetSpritePreviewOptions options = new GetSpritePreviewOptions() {
					SelectedSprite = -1
				};

				DebugSpritePreviewInfo previewInfo = DebugApi.GetSpritePreviewInfo(CpuType, options, _ppuState);
				InitBitmap((int)previewInfo.Width, (int)previewInfo.Height);

				BaseState? ppuState = _ppuState;
				byte[] vram = _vram;
				byte[] spriteRam = _spriteRam;
				UInt32[] palette = _palette.Get().GetRgbPalette();

				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetSpritePreview(CpuType, options, ppuState, vram, spriteRam, palette, framebuffer.FrameBuffer.Address);
				}

				DebugSpriteInfo[] sprites = DebugApi.GetSpriteList(CpuType, options, ppuState, vram, spriteRam, palette);
				InitPreviews(sprites, previewInfo);

				if(Config.ShowOutline) {
					List<Rect> spriteRects = new List<Rect>();
					foreach(SpritePreviewModel sprite in SpritePreviews) {
						spriteRects.Add(sprite.GetPreviewRect());
					}
					SpriteRects = spriteRects;
				} else {
					SpriteRects = null;
				}

				int selectedIndex = SelectedSprite?.SpriteIndex ?? -1;
				if(selectedIndex >= 0 && selectedIndex < SpritePreviews.Count) {
					SelectedSprite = SpritePreviews[selectedIndex];
					UpdateSelectionPreview();
				} else {
					SelectedSprite = null;
				}

				ListView.RefreshList();

				UpdateTooltips();
				UpdateSelection(SelectedSprite);

				UpdateMouseOverRect();
			});
		}

		private void UpdateSelectionPreview()
		{
			if(SelectedSprite != null) {
				SelectedPreviewPanel = GetPreviewPanel(SelectedSprite, SelectedPreviewPanel);
			} else {
				SelectedPreviewPanel = null;
			}
		}

		private void UpdateMouseOverRect()
		{
			if(PreviewPanelSprite != null) {
				MouseOverRect = PreviewPanelSprite.GetPreviewRect();
			} else if(ViewerMousePos != null && GetMatchingSprite(ViewerMousePos.Value) is SpritePreviewModel sprite) {
				MouseOverRect = sprite.GetPreviewRect();
			} else {
				MouseOverRect = null;
			}
		}

		private void UpdateTooltips()
		{
			if(PreviewPanelSprite != null && PreviewPanelTooltip != null) {
				GetPreviewPanel(PreviewPanelSprite, PreviewPanelTooltip);
			} else if(ViewerMousePos != null && ViewerTooltip != null) {
				SpritePreviewModel? sprite = GetMatchingSprite(ViewerMousePos.Value);
				if(sprite != null) {
					GetPreviewPanel(sprite, ViewerTooltip);
				}
			}
		}

		public void UpdateSelection(SpritePreviewModel? sprite)
		{
			if(sprite != null) {
				SelectionRect = sprite.GetPreviewRect();
			} else {
				SelectionRect = Rect.Empty;
			}
		}

		public SpritePreviewModel? GetMatchingSprite(PixelPoint p)
		{
			Point point = p.ToPoint(1);
			for(int i = SpritePreviews.Count - 1; i >= 0; i--) {
				SpritePreviewModel sprite = SpritePreviews[i];
				if(sprite.GetPreviewRect().Contains(point)) {
					return sprite;
				}
			}

			return null;
		}
	}
}
