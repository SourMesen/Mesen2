using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
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
	public class TilemapViewerViewModel : DisposableViewModel, ICpuTypeModel, IMouseOverViewerModel
	{
		[Reactive] public CpuType CpuType { get; set; }
		[Reactive] public bool IsNes { get; private set; }

		public TilemapViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public Rect SelectionRect { get; set; }
		[Reactive] public int GridSizeX { get; set; } = 8;
		[Reactive] public int GridSizeY { get; set; } = 8;

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public DynamicTooltip TilemapInfoPanel { get; private set; } = new DynamicTooltip();
		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }
		
		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; private set; } = new List<TilemapViewerTab>();
		[Reactive] public bool ShowTabs { get; private set; }
		[Reactive] public TilemapViewerTab SelectedTab { get; set; }

		[Reactive] public Rect ScrollOverlayRect { get; private set; } = Rect.Empty;
		
		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();
		
		private DebugTilemapInfo _tilemapInfo;
		private PictureViewer _picViewer;
		private BaseState? _ppuState;
		private byte[]? _prevVram;
		private byte[] _vram = Array.Empty<byte>();
		private UInt32[] _palette = Array.Empty<UInt32>();
		private bool _refreshDataOnTabChange;

		[Obsolete("For designer only")]
		public TilemapViewerViewModel() : this(CpuType.Snes, new PictureViewer(), null) { }

		public TilemapViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TilemapViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);

			CpuType = cpuType;

			_picViewer = picViewer;
			InitForCpuType();

			SelectedTab = Tabs[0];

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
					OnClick = () => _picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => _picViewer.ZoomOut()
				},
			});

			DebugShortcutManager.CreateContextMenu(picViewer, new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_ViewInMemoryViewer),
					OnClick = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						if(tile != null && tile.Value.TileMapAddress >= 0) {
							MemoryToolsWindow.ShowInMemoryTools(GetVramMemoryType(), tile.Value.TileMapAddress);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ViewInTileViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_ViewInTileViewer),
					OnClick = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						if(tile != null && tile.Value.TileAddress >= 0) {
							TileViewerWindow.OpenAtTile(CpuType, GetVramMemoryType(), tile.Value.TileAddress, _tilemapInfo.Format, TileLayout.Normal, tile.Value.PaletteIndex);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					OnClick = () => _picViewer.ExportToPng()
				}
			});

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			AddDisposable(this.WhenAnyValue(x => x.Tabs).Subscribe(x => ShowTabs = x.Count > 1));
			AddDisposable(this.WhenAnyValue(x => x.SelectedTab).Subscribe(x => {
				if(_refreshDataOnTabChange) {
					RefreshData();
				} else {
					RefreshTab();
				}
			}));
			AddDisposable(this.WhenAnyValue(x => x.SelectionRect).Subscribe(x => UpdatePreviewPanel()));
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, Config_PropertyChanged));

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		private void InitForCpuType()
		{
			_refreshDataOnTabChange = false;
			switch(CpuType) {
				case CpuType.Snes:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "Layer 1", Layer = 0 },
						new() { Title = "Layer 2", Layer = 1 },
						new() { Title = "Layer 3", Layer = 2 },
						new() { Title = "Layer 4", Layer = 3 },
					};
					break;

				case CpuType.Nes:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "", Layer = 0 }
					};
					break;

				case CpuType.Pce:
					if(DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine).IsSuperGrafx) {
						_refreshDataOnTabChange = true;
						Tabs = new List<TilemapViewerTab>() {
							new() { Title = "VDC1", Layer = 0 },
							new() { Title = "VDC2", Layer = 1, VramMemoryType = MemoryType.PceVideoRamVdc2 }
						};
					} else {
						Tabs = new List<TilemapViewerTab>() {
							new() { Title = "", Layer = 0 }
						};
					}
					break;

				case CpuType.Gameboy:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "$9800", Layer = 0 },
						new() { Title = "$9C00", Layer = 1 }
					};
					break;
			}
		}

		private DebugTilemapTileInfo? GetSelectedTileInfo()
		{
			if(_ppuState == null || _prevVram == null) {
				return null;
			} else {
				PixelPoint p;
				if(ViewerMousePos.HasValue) {
					p = ViewerMousePos.Value;
				} else {
					if(SelectionRect.IsEmpty) {
						return null;
					}
					p = PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
				}
				return DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _prevVram, _ppuState);
			}
		}

		private void UpdatePreviewPanel()
		{
			if(SelectionRect.IsEmpty) {
				PreviewPanel = null;
			} else {
				PreviewPanel = GetPreviewPanel(PixelPoint.FromPoint(SelectionRect.TopLeft, 1), PreviewPanel);
			}

			if(ViewerTooltip != null && ViewerMousePos != null) {
				GetPreviewPanel(ViewerMousePos.Value, ViewerTooltip);
			}
		}

		private void Config_PropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			RefreshTab();
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap(int width, int height)
		{
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		private GetTilemapOptions GetOptions(TilemapViewerTab tab, byte[]? prevVram = null)
		{
			return new GetTilemapOptions() {
				Layer = (byte)tab.Layer,
				CompareVram = prevVram,
				HighlightTileChanges = Config.HighlightTileChanges,
				HighlightAttributeChanges = Config.HighlightAttributeChanges,
				DisplayMode = Config.DisplayMode
			};
		}

		private MemoryType GetVramMemoryType()
		{
			return SelectedTab?.VramMemoryType ?? CpuType.GetVramMemoryType();
		}

		public void RefreshData()
		{
			BaseState ppuState = DebugApi.GetPpuState(CpuType);
			_ppuState = ppuState;
			_prevVram = _vram;
			_vram = DebugApi.GetMemoryState(GetVramMemoryType());
			_palette = DebugApi.GetPaletteInfo(CpuType).GetRgbPalette();

			RefreshTab();
		}

		private void RefreshTab()
		{
			Dispatcher.UIThread.Post(() => {
				if(_ppuState == null) {
					return;
				}

				BaseState ppuState = _ppuState;
				byte[]? prevVram = _prevVram;
				byte[] vram = _vram;
				uint[] palette = _palette;

				GetTilemapOptions options;
				FrameInfo size;

				foreach(TilemapViewerTab tab in Tabs) {
					options = GetOptions(tab);
					size = DebugApi.GetTilemapSize(CpuType, options, ppuState);
					tab.Enabled = size.Width != 0 && size.Height != 0;
				}

				if(!SelectedTab.Enabled) {
					SelectedTab = Tabs[0];
				}

				options = GetOptions(SelectedTab, prevVram);
				size = DebugApi.GetTilemapSize(CpuType, options, ppuState);
				InitBitmap((int)size.Width, (int)size.Height);

				using(var framebuffer = ViewerBitmap.Lock()) {
					_tilemapInfo = DebugApi.GetTilemap(CpuType, options, ppuState, vram, palette, framebuffer.FrameBuffer.Address);
				}

				if(_tilemapInfo.Bpp == 0) {
					return;
				}

				GridSizeX = (int)_tilemapInfo.TileWidth;
				GridSizeY = (int)_tilemapInfo.TileHeight;

				UpdatePreviewPanel();
				UpdateTilemapInfo();

				if(Config.ShowScrollOverlay) {
					ScrollOverlayRect = new Rect(
						_tilemapInfo.ScrollX % size.Width,
						_tilemapInfo.ScrollY % size.Height,
						_tilemapInfo.ScrollWidth,
						_tilemapInfo.ScrollHeight
					);
				} else {
					ScrollOverlayRect = Rect.Empty;
				}
			});
		}

		private void UpdateTilemapInfo()
		{
			TooltipEntries entries = TilemapInfoPanel.Items ?? new TooltipEntries();
			DebugTilemapInfo info = _tilemapInfo;
			entries.AddEntry("Size", info.ColumnCount + "x" + info.RowCount);
			entries.AddEntry("Size (px)", info.ColumnCount* info.TileWidth + "x" + info.RowCount* info.TileHeight);
			entries.AddEntry("Tilemap Address", "$" + info.TilemapAddress.ToString("X4"));
			entries.AddEntry("Tileset Address", "$" + info.TilesetAddress.ToString("X4"));
			entries.AddEntry("BPP", info.Bpp);
			TilemapInfoPanel.Items = entries;
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			if(_ppuState == null || _prevVram == null) {
				return null;
			}

			DebugTilemapTileInfo? result = DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _prevVram, _ppuState);
			if(result == null) {
				return null;
			}

			DebugTilemapTileInfo tileInfo = result.Value;

			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			PixelRect cropRect = new PixelRect(p.X / tileInfo.Width * tileInfo.Width, p.Y / tileInfo.Height * tileInfo.Height, tileInfo.Width, tileInfo.Height);

			entries.StartUpdate();

			entries.AddPicture("Tile", ViewerBitmap, 6, cropRect);

			if(_tilemapInfo.Bpp <= 4) {
				int paletteSize = (int)Math.Pow(2, _tilemapInfo.Bpp);
				int paletteIndex = tileInfo.PaletteIndex >= 0 ? tileInfo.PaletteIndex : 0;
				UInt32[] tilePalette = new UInt32[paletteSize];
				Array.Copy(_palette, paletteIndex * paletteSize, tilePalette, 0, paletteSize);

				entries.AddEntry("Palette", tilePalette);
			}

			entries.AddEntry("Column, Row", tileInfo.Column + ", " + tileInfo.Row);
			entries.AddEntry("Size", tileInfo.Width + "x" + tileInfo.Height);

			if(tileInfo.TileMapAddress >= 0) {
				entries.AddEntry("Tilemap address", "$" + tileInfo.TileMapAddress.ToString("X4"));
			}
			if(tileInfo.TileIndex >= 0) {
				entries.AddEntry("Tile index", "$" + tileInfo.TileIndex.ToString("X2"));
			}
			if(tileInfo.TileAddress >= 0) {
				entries.AddEntry("Tile address", "$" + tileInfo.TileAddress.ToString("X4"));
			}
			if(tileInfo.PaletteIndex >= 0) {
				entries.AddEntry("Palette index", tileInfo.PaletteIndex.ToString());
			}
			if(tileInfo.PaletteAddress >= 0) {
				entries.AddEntry("Palette address", "$" + tileInfo.PaletteAddress.ToString("X2"));
			}
			if(tileInfo.AttributeAddress >= 0) {
				entries.AddEntry("Attribute address", "$" + tileInfo.AttributeAddress.ToString("X4"));
			}
			if(tileInfo.HorizontalMirroring != NullableBoolean.Undefined) {
				entries.AddEntry("Horizontal mirror", tileInfo.HorizontalMirroring == NullableBoolean.True);
			}
			if(tileInfo.VerticalMirroring != NullableBoolean.Undefined) {
				entries.AddEntry("Vertical mirror", tileInfo.VerticalMirroring == NullableBoolean.True);
			}
			if(tileInfo.HighPriority != NullableBoolean.Undefined) {
				entries.AddEntry("High priority", tileInfo.HighPriority == NullableBoolean.True);
			}

			entries.EndUpdate();

			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}

		public void OnGameLoaded()
		{
			InitForCpuType();
			IsNes = CpuType == CpuType.Nes;
			RefreshData();
		}
	}

	public class TilemapViewerTab : ViewModelBase
	{
		[Reactive] public string Title { get; set; } = "";
		[Reactive] public int Layer { get; set; }  = 0;
		[Reactive] public MemoryType? VramMemoryType { get; set; }
		[Reactive] public bool Enabled { get; set; } = true;
	}
}
