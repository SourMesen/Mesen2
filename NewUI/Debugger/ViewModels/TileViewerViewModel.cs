using Avalonia;
using Avalonia.Controls;
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
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : DisposableViewModel, ICpuTypeModel, IMouseOverViewerModel
	{
		[Reactive] public CpuType CpuType { get; set; }

		public TileViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }

		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
		[Reactive] public PaletteSelectionMode PaletteSelectionMode { get; private set; }
		[Reactive] public int PaletteColumnCount { get; private set; } = 16;

		[Reactive] public int AddressIncrement { get; private set; }
		[Reactive] public int MaximumAddress { get; private set; } = int.MaxValue;

		[Reactive] public int GridSizeX { get; set; } = 8;
		[Reactive] public int GridSizeY { get; set; } = 8;

		[Reactive] public Rect SelectionRect { get; set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();
		[Reactive] public Enum[] AvailableFormats { get; set; } = Array.Empty<Enum>();
		[Reactive] public bool ShowFormatDropdown { get; set; }

		[Reactive] public List<ConfigPreset> ConfigPresets { get; set; } = new List<ConfigPreset>();

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private BaseState? _ppuState;
		private byte[] _sourceData = Array.Empty<byte>();

		[Obsolete("For designer only")]
		public TileViewerViewModel() : this(CpuType.Snes, new PictureViewer(), null) { }

		public TileViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TileViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;

			InitBitmap();

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

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
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TileViewer_ViewInMemoryViewer),
					IsEnabled = () => GetSelectedTileAddress() >= 0,
					OnClick = () => {
						int address = GetSelectedTileAddress();
						if(address >= 0) {
							MemoryToolsWindow.ShowInMemoryTools(Config.Source, address);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					OnClick = () => picViewer.ExportToPng()
				}
			});

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			InitForCpuType();

			AddDisposable(this.WhenAnyValue(x => x.Config.Format).Subscribe(x => {
				PaletteSelectionMode = GetBitsPerPixel(x) switch {
					2 => PaletteSelectionMode.FourColors,
					4 => PaletteSelectionMode.SixteenColors,
					_ => PaletteSelectionMode.None
				};

				PixelSize tileSize = GetTileSize(x);
				if(GridSizeX != tileSize.Width || GridSizeY != tileSize.Height) {
					GridSizeX = tileSize.Width;
					GridSizeY = tileSize.Height;
					SelectionRect = Rect.Empty;
					PreviewPanel = null;
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Layout).Subscribe(x => {
				ApplyColumnRowCountRestrictions();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.ColumnCount, x => x.Config.RowCount, x => x.Config.Format).Subscribe(x => {
				ApplyColumnRowCountRestrictions();
				AddressIncrement = Config.ColumnCount * Config.RowCount * 8 * 8 * GetBitsPerPixel(Config.Format) / 8;
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Source).Subscribe(memType => {
				MaximumAddress = Math.Max(0, DebugApi.GetMemorySize(memType) - 1);
				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.SelectionRect).Subscribe(x => UpdatePreviewPanel()));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, Config_PropertyChanged));
		}

		private void ApplyColumnRowCountRestrictions()
		{
			if(Config.Layout != TileLayout.Normal || Config.Format == TileFormat.PceSpriteBpp4) {
				//Force multiple of 2 when in 8x16 or 16x16 display modes
				Config.ColumnCount &= ~0x01;
				Config.RowCount &= ~0x01;
			}
		}

		private void InitForCpuType()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsTileViewer() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.Source)) {
				Config.Source = CpuType.GetVramMemoryType();
				Config.StartAddress = 0;
			}

			AvailableFormats = CpuType switch {
				CpuType.Snes => new Enum[] { TileFormat.Bpp2, TileFormat.Bpp4, TileFormat.Bpp8, TileFormat.DirectColor, TileFormat.Mode7, TileFormat.Mode7ExtBg, TileFormat.Mode7DirectColor },
				CpuType.Nes => new Enum[] { TileFormat.NesBpp2 },
				CpuType.Gameboy => new Enum[] { TileFormat.Bpp2 },
				CpuType.Pce => new Enum[] { TileFormat.Bpp4, TileFormat.PceSpriteBpp4 }, //TODO
				_ => throw new Exception("Unsupported CPU type")
			};

			ConfigPresets = GetConfigPresets();

			if(AvailableFormats.Contains(Config.Format)) {
				Config.Format = (TileFormat)AvailableFormats[0];
			}
			ShowFormatDropdown = AvailableFormats.Length > 1;
		}

		private List<ConfigPreset> GetConfigPresets()
		{
			switch(CpuType) {
				case CpuType.Snes:
					return new() {
						new("BG1", () => ApplyBgPreset(0)),
						new("BG2", () => ApplyBgPreset(1)),
						new("BG3", () => ApplyBgPreset(2)),
						new("BG4", () => ApplyBgPreset(3)),
						new("OAM1", () => ApplySpritePreset(0)),
						new("OAM2", () => ApplySpritePreset(1)),
						new("PPU", () => ApplyPpuPreset()),
						new("ROM", () => ApplyPrgPreset()),
					};

				case CpuType.Nes:
					return new() {
						new("BG", () => ApplyBgPreset(0)),
						new("OAM", () => ApplySpritePreset(0)),
						new("PPU", () => ApplyPpuPreset()),
						new("CHR", () => ApplyChrPreset()),
						new("ROM", () => ApplyPrgPreset()),
					};

				case CpuType.Gameboy:
					if(DebugApi.GetPpuState<GbPpuState>(CpuType.Gameboy).CgbEnabled) {
						return new() {
							new("BG1", () => ApplyBgPreset(0)),
							new("BG2", () => ApplyBgPreset(1)),
							new("OAM1", () => ApplySpritePreset(0)),
							new("OAM2", () => ApplySpritePreset(1)),
							new("PPU", () => ApplyPpuPreset()),
							new("ROM", () => ApplyPrgPreset()),
						};
					} else {
						return new() {
							new("BG", () => ApplyBgPreset(0)),
							new("OAM", () => ApplySpritePreset(0)),
							new("PPU", () => ApplyPpuPreset()),
							new("ROM", () => ApplyPrgPreset()),
						};
					}

				case CpuType.Pce:
					if(DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine).IsSuperGrafx) {
						return new() {
							new("BG1", () => ApplyBgPreset(0)),
							new("SPR1", () => ApplySpritePreset(0)),
							new("BG2", () => ApplyBgPreset(1)),
							new("SPR2", () => ApplySpritePreset(1)),
						};
					} else {
						return new() {
							new("BG", () => ApplyBgPreset(0)),
							new("Sprites", () => ApplySpritePreset(0)),
						};
					}

				default:
					throw new Exception("Unsupported CPU type");
			}
		}

		private void ApplyPrgPreset()
		{
			Config.Source = CpuType.GetPrgRomMemoryType();
			Config.StartAddress = 0;
			Config.ColumnCount = 16;
			Config.RowCount = 32;
			Config.Layout = TileLayout.Normal;
			Config.Format = (TileFormat)AvailableFormats[0];
		}

		private void ApplyChrPreset()
		{
			Config.Source = DebugApi.GetMemorySize(MemoryType.NesChrRam) > 0 ? MemoryType.NesChrRam : MemoryType.NesChrRom;
			Config.StartAddress = 0;
			Config.ColumnCount = 16;
			Config.RowCount = 32;
			Config.Layout = TileLayout.Normal;
			Config.Format = TileFormat.NesBpp2;
		}

		private void ApplyPpuPreset()
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return;
			}

			switch(CpuType) {
				case CpuType.Snes: {
					Config.Source = MemoryType.SnesVideoRam;
					Config.StartAddress = 0;
					Config.ColumnCount = 16;
					Config.RowCount = 32;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.Bpp4;
					break;
				}

				case CpuType.Nes: {
					Config.Source = MemoryType.NesPpuMemory;
					Config.StartAddress = 0;
					Config.ColumnCount = 16;
					Config.RowCount = 32;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.NesBpp2;
					break;
				}

				case CpuType.Gameboy: {
					Config.Source = MemoryType.GbVideoRam;
					Config.StartAddress = 0;
					Config.ColumnCount = 16;
					Config.RowCount = 32;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.Bpp2;
					break;
				}
			}
		}

		private void ApplyBgPreset(int layer)
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return;
			}

			switch(CpuType) {
				case CpuType.Snes: {
					int[,] layerBpp = new int[8, 4] { { 2, 2, 2, 2 }, { 4, 4, 2, 0 }, { 4, 4, 0, 0 }, { 8, 4, 0, 0 }, { 8, 2, 0, 0 }, { 4, 2, 0, 0 }, { 4, 0, 0, 0 }, { 8, 0, 0, 0 } };
					SnesPpuState ppu = (SnesPpuState)state;
					Config.Source = MemoryType.SnesVideoRam;
					Config.StartAddress = ppu.Layers[layer].ChrAddress * 2;
					Config.ColumnCount = 16;
					Config.RowCount = 16;
					Config.Layout = TileLayout.Normal;
					if(ppu.BgMode == 7) {
						Config.Format = ppu.DirectColorMode ? TileFormat.Mode7DirectColor : TileFormat.Mode7;
						Config.SelectedPalette = 0;
					} else {
						Config.Format = layerBpp[ppu.BgMode, layer] switch {
							2 => TileFormat.Bpp2,
							4 => TileFormat.Bpp4,
							8 => ppu.DirectColorMode ? TileFormat.DirectColor : TileFormat.Bpp8,
							_ => TileFormat.Bpp2
						};

						if(layerBpp[ppu.BgMode, layer] == 8 || Config.SelectedPalette >= (layerBpp[ppu.BgMode, layer] == 2 ? 32 : 8)) {
							Config.SelectedPalette = 0;
						}
					}
					break;
				}

				case CpuType.Nes: {
					NesPpuState ppu = (NesPpuState)state;
					Config.Source = MemoryType.NesPpuMemory;
					Config.StartAddress = ppu.Control.BackgroundPatternAddr;
					Config.ColumnCount = 16;
					Config.RowCount = 16;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.NesBpp2;
					if(Config.SelectedPalette >= 4) {
						Config.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = (GbPpuState)state;
					Config.Source = MemoryType.GbVideoRam;
					Config.StartAddress = (layer == 0 ? 0 : 0x2000) | (ppu.BgTileSelect ? 0 : 0x800);
					Config.ColumnCount = 16;
					Config.RowCount = 16;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.Bpp2;
					Config.Background = ppu.CgbEnabled ? TileBackground.PaletteColor : TileBackground.Default;
					if(!ppu.CgbEnabled || Config.SelectedPalette > 8) {
						Config.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Pce: {
					Config.Source = layer == 0 ?MemoryType.PceVideoRam : MemoryType.PceVideoRamVdc2;
					Config.StartAddress = 0;
					Config.ColumnCount = 32;
					Config.RowCount = 64;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.Bpp4;
					Config.Background = TileBackground.Default;
					if(Config.SelectedPalette >= 16) {
						Config.SelectedPalette = 0;
					}
					break;
				}
			}
		}

		private void ApplySpritePreset(int layer)
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return;
			}

			switch(CpuType) {
				case CpuType.Snes: {
					SnesPpuState ppu = (SnesPpuState)state;
					Config.Source = MemoryType.SnesVideoRam;
					Config.Format = TileFormat.Bpp4;
					Config.ColumnCount = 16;
					Config.RowCount = 16;
					Config.StartAddress = (ppu.OamBaseAddress + (layer == 1 ? ppu.OamAddressOffset : 0)) * 2;
					if(Config.SelectedPalette < 8 || Config.SelectedPalette >= 16) {
						Config.SelectedPalette = 8;
					}
					break;
				}

				case CpuType.Nes: {
					NesPpuState ppu = (NesPpuState)state;
					if(ppu.Control.LargeSprites) {
						Config.StartAddress = 0;
						Config.ColumnCount = 16;
						Config.RowCount = 32;
						Config.Layout = TileLayout.SingleLine8x16;
					} else {
						Config.StartAddress = ppu.Control.SpritePatternAddr;
						Config.ColumnCount = 16;
						Config.RowCount = 16;
						Config.Layout = TileLayout.Normal;
					}
					Config.Source = MemoryType.NesPpuMemory;
					if(Config.SelectedPalette < 4 || Config.SelectedPalette >= 8) {
						Config.SelectedPalette = 4;
					}
					Config.Format = TileFormat.NesBpp2;
					break;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = (GbPpuState)state;
					Config.Source = MemoryType.GbVideoRam;
					Config.StartAddress = layer == 0 ? 0 : 0x2000;
					Config.ColumnCount = 16;
					Config.RowCount = 16;
					Config.Layout = TileLayout.Normal;
					Config.Background = TileBackground.Black;
					Config.Format = TileFormat.Bpp2;
					if(ppu.CgbEnabled && Config.SelectedPalette < 8) {
						Config.SelectedPalette = 8;
					} else if(!ppu.CgbEnabled && Config.SelectedPalette == 0) {
						Config.SelectedPalette = 1;
					}
					break;
				}

				case CpuType.Pce: {
					Config.Source = layer == 0 ? MemoryType.PceVideoRam : MemoryType.PceVideoRamVdc2;
					Config.StartAddress = 0;
					Config.ColumnCount = 32;
					Config.RowCount = 64;
					Config.Layout = TileLayout.Normal;
					Config.Format = TileFormat.PceSpriteBpp4;
					Config.Background = TileBackground.Default;
					if(Config.SelectedPalette < 16) {
						Config.SelectedPalette = 16;
					}
					break;
				}
			}
		}

		public void SelectTile(MemoryType type, int address, TileFormat format, TileLayout layout, int paletteIndex)
		{
			Config.Source = type;
			Config.Format = format;
			Config.SelectedPalette = paletteIndex;
			Config.StartAddress = address / AddressIncrement * AddressIncrement;
			Config.Layout = layout;
			int bitsPerPixel = GetBitsPerPixel(Config.Format);
			PixelSize tileSize = GetTileSize(Config.Format);
			int bytesPerTile = tileSize.Width * tileSize.Height * bitsPerPixel / 8;

			int gap = address - Config.StartAddress;
			int tileNumber = gap / bytesPerTile;
			int tilesPerRow = Config.ColumnCount * 8 / tileSize.Width;

			PixelPoint pos = new PixelPoint(tileNumber % tilesPerRow, tileNumber / tilesPerRow);
			pos = ToLayoutCoordinates(Config.Layout, pos);
			SelectionRect = new Rect(pos.X * tileSize.Width, pos.Y * tileSize.Height, tileSize.Width, tileSize.Height);
		}

		private PixelPoint ToLayoutCoordinates(TileLayout layout, PixelPoint pos)
		{
			int column = pos.X;
			int row = pos.Y;

			switch(layout) {
				case TileLayout.SingleLine8x16: {
					int displayColumn = column / 2 + ((row & 0x01) != 0 ? Config.ColumnCount / 2 : 0);
					int displayRow = (row & ~0x01) + ((column & 0x01) != 0 ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.SingleLine16x16: {
					int displayColumn = (column / 2) + (column & 0x01) + ((row & 0x01) != 0 ? Config.ColumnCount / 2 : 0) + ((column & 0x02) != 0 ? -1 : 0);
					int displayRow = (row & ~0x01) + ((column & 0x02) != 0 ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.Normal:
					return pos;

				default:
					throw new NotImplementedException("TileLayout not supported");
			}
		}

		private PixelPoint FromLayoutCoordinates(TileLayout layout, PixelPoint pos)
		{
			int column = pos.X;
			int row = pos.Y;

			switch(layout) {
				case TileLayout.SingleLine8x16: {
					//A0 B0 C0 D0 -> A0 A1 B0 B1
					//A1 B1 C1 D1    C0 C1 D0 D1
					int displayColumn = (column * 2) % Config.ColumnCount + (row & 0x01);
					int displayRow = (row & ~0x01) + ((column >= Config.ColumnCount / 2) ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.SingleLine16x16: {
					//A0 A1 B0 B1 C0 C1 D0 D1 -> A0 A1 A2 A3 B0 B1 B2 B3
					//A2 A3 B2 B3 C2 C3 D2 D3    C0 C1 C2 C3 D0 D1 D2 D3
					//E0 E1 F0 F1 G0 G1 H0 H1 -> E0 E1 E2 E3 F0 F1 F2 F3
					//E2 E3 F2 F3 G2 G3 H2 H3    G0 G1 G2 G3 H0 H1 H2 H3
					int displayColumn = ((column & ~0x01) * 2 + ((row & 0x01) != 0 ? 2 : 0) + (column & 0x01)) % Config.ColumnCount;
					int displayRow = (row & ~0x01) + ((column >= Config.ColumnCount / 2) ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.Normal:
					return pos;

				default:
					throw new NotImplementedException("TileLayout not supported");
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

		private void Config_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			RefreshTab();
		}

		public void RefreshData()
		{
			_ppuState = DebugApi.GetPpuState(CpuType);

			PaletteColors = DebugApi.GetPaletteInfo(CpuType).GetRgbPalette();
			PaletteColumnCount = PaletteColors.Length > 16 ? 16 : 4;
			_sourceData = DebugApi.GetMemoryState(Config.Source);

			RefreshTab();
		}

		private void RefreshTab()
		{
			Dispatcher.UIThread.Post(() => {
				InitBitmap();

				using(var framebuffer = ViewerBitmap.Lock()) {
					DebugApi.GetTileView(CpuType, GetOptions(), _sourceData, _sourceData.Length, PaletteColors, framebuffer.FrameBuffer.Address);
				}

				UpdatePreviewPanel();
			});
		}

		private int GetTileAddress(PixelPoint pixelPosition)
		{
			int bitsPerPixel = GetBitsPerPixel(Config.Format);
			PixelSize tileSize = GetTileSize(Config.Format);
			int bytesPerTile = tileSize.Width * tileSize.Height * bitsPerPixel / 8;
			PixelPoint pos = FromLayoutCoordinates(Config.Layout, new PixelPoint(pixelPosition.X / tileSize.Width, pixelPosition.Y / tileSize.Height));
			int offset = (pos.Y * Config.ColumnCount * 8 / tileSize.Width + pos.X) * bytesPerTile;
			return (Config.StartAddress + offset) % (MaximumAddress + 1);
		}

		private int GetSelectedTileAddress()
		{
			PixelPoint p;
			if(ViewerMousePos.HasValue) {
				p = ViewerMousePos.Value;
			} else {
				if(SelectionRect.IsEmpty) {
					return -1;
				}
				p = PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
			}

			return GetTileAddress(p);
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			TooltipEntries entries = tooltipToUpdate?.Items ?? new();

			entries.StartUpdate();

			PixelSize tileSize = GetTileSize(Config.Format);
			PixelRect cropRect = new PixelRect(p.X / tileSize.Width * tileSize.Width, p.Y / tileSize.Height * tileSize.Height, tileSize.Width, tileSize.Height);
			entries.AddPicture("Tile", ViewerBitmap, 6, cropRect);

			int address = GetTileAddress(cropRect.TopLeft);
			entries.AddEntry("Tile address", "$" + address.ToString("X4"));

			entries.EndUpdate();

			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}

		private GetTileViewOptions GetOptions()
		{
			return new GetTileViewOptions() {
				Format = Config.Format,
				Width = Config.ColumnCount,
				Height = Config.RowCount,
				Palette = Config.SelectedPalette,
				Layout = Config.Layout,
				StartAddress = Config.StartAddress,
				Background = Config.Background,
				UseGrayscalePalette = Config.UseGrayscalePalette
			};
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap()
		{
			int width = Config.ColumnCount * 8;
			int height = Config.RowCount * 8;
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		private int GetBitsPerPixel(TileFormat format)
		{
			return format switch {
				TileFormat.Bpp2 => 2,
				TileFormat.Bpp4 => 4,
				TileFormat.DirectColor => 8,
				TileFormat.Mode7 => 16,
				TileFormat.Mode7DirectColor => 16,
				TileFormat.NesBpp2 => 2,
				TileFormat.PceSpriteBpp4 => 4,
				_ => 8,
			};
		}

		private PixelSize GetTileSize(TileFormat format)
		{
			return format switch {
				TileFormat.PceSpriteBpp4 => new PixelSize(16, 16),
				_ => new PixelSize(8,8),
			};
		}

		public void OnGameLoaded()
		{
			InitForCpuType();
			RefreshData();
		}
	}

	public class ConfigPreset
	{
		public string Name { get; }
		public ReactiveCommand<Unit, Unit> ClickCommand { get; }
		private Action _applyPreset;

		public ConfigPreset(string name, Action applyPreset)
		{
			Name = name;
			_applyPreset = applyPreset;
			ClickCommand = ReactiveCommand.Create(_applyPreset);
		}
	}
}
