using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
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
		public CpuType CpuType { get; set; }

		public TileViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }

		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();
		[Reactive] public UInt32[] RawPalette { get; set; } = Array.Empty<UInt32>();
		[Reactive] public RawPaletteFormat RawFormat { get; set; }
		[Reactive] public PaletteSelectionMode PaletteSelectionMode { get; private set; }
		[Reactive] public int PaletteColumnCount { get; private set; } = 16;
		[Reactive] public int SelectedPalette { get; set; } = 0;

		[Reactive] public int AddressIncrement { get; private set; }
		[Reactive] public int MaximumAddress { get; private set; } = int.MaxValue;

		[Reactive] public int GridSizeX { get; set; } = 8;
		[Reactive] public int GridSizeY { get; set; } = 8;

		[Reactive] public Rect SelectionRect { get; set; }
		
		[Reactive] public List<PictureViewerLine>? PageDelimiters { get; set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();
		[Reactive] public Enum[] AvailableFormats { get; set; } = Array.Empty<Enum>();
		[Reactive] public bool ShowFormatDropdown { get; set; }
		[Reactive] public bool ShowFilterDropdown { get; set; }

		[Reactive] public List<List<ConfigPreset>> ConfigPresetRows { get; set; } = new() { new(), new(), new() };
		[Reactive] public List<ConfigPreset> ConfigPresets { get; set; } = new List<ConfigPreset>();

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		public int ColumnCount => Math.Clamp(Config.ColumnCount, 4, 256); 
		public int RowCount => Math.Clamp(Config.RowCount, 4, 256);

		private BaseState? _ppuState;
		private object _updateLock = new();
		private byte[] _coreSourceData = Array.Empty<byte>();
		private byte[] _sourceData = Array.Empty<byte>();
		private bool _refreshPending;

		[Obsolete("For designer only")]
		public TileViewerViewModel() : this(CpuType.Snes, new PictureViewer(), null) { }

		public TileViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TileViewer.Clone();
			CpuType = cpuType;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming, cpuType);

			InitBitmap();

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			FileMenuActions = AddDisposables(new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveAsPng),
					OnClick = () => picViewer.ExportToPng()
				},
				new ContextMenuSeparator(),
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
					ActionType = ActionType.ShowSettingsPanel,
					Shortcut =  () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ToggleSettingsPanel),
					IsSelected = () => Config.ShowSettingsPanel,
					OnClick = () => Config.ShowSettingsPanel = !Config.ShowSettingsPanel
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

			AddDisposables(DebugShortcutManager.CreateContextMenu(picViewer, new List<object> {
				new ContextMenuAction() {
					ActionType = ActionType.EditTile,
					HintText = () => $"{GridSizeX}px x {GridSizeY}px",
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TileViewer_EditTile),
					IsEnabled = () => GetSelectedTileAddress() >= 0,
					OnClick = () => EditTileGrid(1, 1, wnd)
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditTiles,
					SubActions = new() {
						GetEditTileAction(1, 2, wnd),
						GetEditTileAction(2, 1, wnd),
						GetEditTileAction(2, 2, wnd),
						GetEditTileAction(4, 4, wnd),
						GetEditTileAction(8, 8, wnd)
					}
				},
				new ContextMenuSeparator(),
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
				new ContextMenuSeparator() { IsVisible = () => CpuType == CpuType.Nes },
				new ContextMenuAction() {
					ActionType = ActionType.CopyToHdPackFormat,
					IsVisible = () => CpuType == CpuType.Nes,
					IsEnabled = () => GetSelectedTileAddress() >= 0 && HdPackCopyHelper.IsActionAllowed(Config.Source),
					OnClick = () => {
						int address = GetSelectedTileAddress();
						if(address >= 0) {
							HdPackCopyHelper.CopyToHdPackFormat(address, Config.Source, RawPalette, SelectedPalette, false);
						}
					}
				}
			}));

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);

			InitForCpuType();

			AddDisposable(this.WhenAnyValue(x => x.Config.Format, x => x.RawPalette).Subscribe(x => {
				PaletteSelectionMode selMode = PaletteSelectionMode;
				selMode = x.Item1.GetBitsPerPixel() switch {
					1 => PaletteSelectionMode.TwoColors,
					2 => PaletteSelectionMode.FourColors,
					4 => PaletteSelectionMode.SixteenColors,
					8 => RawPalette.Length >= 512 ? PaletteSelectionMode._256Colors : PaletteSelectionMode.None,
					_ => PaletteSelectionMode.None
				};

				if(selMode != PaletteSelectionMode) {
					PaletteSelectionMode = selMode;

					PixelSize tileSize = x.Item1.GetTileSize();
					if(GridSizeX != tileSize.Width || GridSizeY != tileSize.Height) {
						GridSizeX = tileSize.Width;
						GridSizeY = tileSize.Height;
						SelectionRect = default;
						PreviewPanel = null;
					}

					RefreshPalette();
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Layout).Subscribe(x => {
				ApplyColumnRowCountRestrictions();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.StartAddress).Subscribe(x => {
				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.ColumnCount, x => x.Config.RowCount, x => x.Config.Format).Subscribe(x => {
				//Enforce min/max values for column/row counts
				Config.ColumnCount = ColumnCount;
				Config.RowCount = RowCount;

				ApplyColumnRowCountRestrictions();
				AddressIncrement = ColumnCount * RowCount * 8 * 8 * Config.Format.GetBitsPerPixel() / 8;

				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Source).Subscribe(memType => {
				MaximumAddress = Math.Max(0, DebugApi.GetMemorySize(memType) - 1);
				if(Config.StartAddress > MaximumAddress) {
					Config.StartAddress = 0;
				}
				ShowFilterDropdown = memType.SupportsCdl();
				RefreshData();
			}));

			AddDisposable(this.WhenAnyValue(x => x.SelectedPalette).Subscribe(x => RefreshTab()));
			AddDisposable(this.WhenAnyValue(x => x.SelectionRect).Subscribe(x => UpdatePreviewPanel()));

			LoadSelectedPreset(false);

			AddDisposable(this.WhenAnyValue(
				x => x.Config.Source, x => x.Config.StartAddress, x => x.Config.ColumnCount,
				x => x.Config.RowCount, x => x.Config.Format
			).Skip(1).Subscribe(x => ClearPresetSelection()));
			
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
			string selectedPreset = Config.SelectedPreset;

			AvailableFormats = CpuType switch {
				CpuType.Snes => new Enum[] { TileFormat.Bpp2, TileFormat.Bpp4, TileFormat.Bpp8, TileFormat.DirectColor, TileFormat.Mode7, TileFormat.Mode7ExtBg, TileFormat.Mode7DirectColor },
				CpuType.Nes => new Enum[] { TileFormat.NesBpp2 },
				CpuType.Gameboy => new Enum[] { TileFormat.Bpp2 },
				CpuType.Pce => new Enum[] { TileFormat.Bpp4, TileFormat.PceSpriteBpp4, TileFormat.PceSpriteBpp2Sp01, TileFormat.PceSpriteBpp2Sp23, TileFormat.PceBackgroundBpp2Cg0, TileFormat.PceBackgroundBpp2Cg1 },
				CpuType.Sms => new Enum[] { TileFormat.SmsBpp4, TileFormat.SmsSgBpp1 },
				CpuType.Gba => new Enum[] { TileFormat.GbaBpp4, TileFormat.GbaBpp8 },
				CpuType.Ws => new Enum[] { TileFormat.Bpp2, TileFormat.SmsBpp4, TileFormat.WsBpp4Packed },
				_ => throw new Exception("Unsupported CPU type")
			};

			ConfigPresets = GetConfigPresets();
			ConfigPresetRows = new(ConfigPresetRows); //Force UI update

			if(!AvailableFormats.Contains(Config.Format)) {
				Config.Format = (TileFormat)AvailableFormats[0];
			}
			ShowFormatDropdown = AvailableFormats.Length > 1;

			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsTileViewer() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.Source)) {
				//Switched to another console, or game doesn't support the same memory type, etc.
				ResetToDefaultView();
			}
			ShowFilterDropdown = Config.Source.SupportsCdl();
			MaximumAddress = Math.Max(0, DebugApi.GetMemorySize(Config.Source) - 1);

			Dispatcher.UIThread.Post(() => {
				Config.SelectedPreset = selectedPreset;
				LoadSelectedPreset(true);
			});
		}

		private void ResetToDefaultView()
		{
			//Reset to the default view for each console (show all of VRAM)
			Config.Source = CpuType.GetVramMemoryType();
			Config.StartAddress = 0;
			switch(CpuType) {
				case CpuType.Snes:
				case CpuType.Nes:
				case CpuType.Gameboy:
					ApplyPpuPreset();
					break;

				case CpuType.Pce:
					ApplyBgPreset(0);
					break;
			}
		}

		public void SelectTile(MemoryType type, int address, TileFormat format, TileLayout layout, int paletteIndex)
		{
			Config.Source = type;
			Config.Format = format;
			SelectedPalette = paletteIndex;
			Config.StartAddress = address / AddressIncrement * AddressIncrement;
			Config.Layout = layout;
			PixelSize tileSize = Config.Format.GetTileSize();
			int bytesPerTile = Config.Format.GetBytesPerTile();

			int gap = address - Config.StartAddress;
			int tileNumber = gap / bytesPerTile;
			int tilesPerRow = ColumnCount * 8 / tileSize.Width;

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
					int displayColumn = column / 2 + ((row & 0x01) != 0 ? ColumnCount / 2 : 0);
					int displayRow = (row & ~0x01) + ((column & 0x01) != 0 ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.SingleLine16x16: {
					int displayColumn = (column / 2) + (column & 0x01) + ((row & 0x01) != 0 ? ColumnCount / 2 : 0) + ((column & 0x02) != 0 ? -1 : 0);
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
					int displayColumn = (column * 2) % ColumnCount + (row & 0x01);
					int displayRow = (row & ~0x01) + ((column >= ColumnCount / 2) ? 1 : 0);
					return new PixelPoint(displayColumn, displayRow);
				}

				case TileLayout.SingleLine16x16: {
					//A0 A1 B0 B1 C0 C1 D0 D1 -> A0 A1 A2 A3 B0 B1 B2 B3
					//A2 A3 B2 B3 C2 C3 D2 D3    C0 C1 C2 C3 D0 D1 D2 D3
					//E0 E1 F0 F1 G0 G1 H0 H1 -> E0 E1 E2 E3 F0 F1 F2 F3
					//E2 E3 F2 F3 G2 G3 H2 H3    G0 G1 G2 G3 H0 H1 H2 H3
					int displayColumn = ((column & ~0x01) * 2 + ((row & 0x01) != 0 ? 2 : 0) + (column & 0x01)) % ColumnCount;
					int displayRow = (row & ~0x01) + ((column >= ColumnCount / 2) ? 1 : 0);
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
			if(SelectionRect == default) {
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
			
			RefreshPalette();

			int bytesPerTile = Config.Format.GetBytesPerTile();
			int tileCount = Config.RowCount * Config.ColumnCount;
			int totalSize = bytesPerTile * tileCount;

			lock(_updateLock) {
				DebugApi.GetMemoryValues(Config.Source, (uint)Config.StartAddress, (uint)(Config.StartAddress + totalSize - 1), ref _coreSourceData);
			}

			RefreshTab();
		}

		private void RefreshPalette()
		{
			DebugPaletteInfo palette = DebugApi.GetPaletteInfo(CpuType, new GetPaletteInfoOptions() { Format = Config.Format });
			var paletteColors = palette.GetRgbPalette();
			var rawPalette = palette.GetRawPalette();
			var rawFormat = palette.RawFormat;
			var paletteColumnCount = paletteColors.Length > 16 ? 16 : 4;

			Dispatcher.UIThread.Post(() => {
				PaletteColors = paletteColors;
				RawPalette = rawPalette;
				RawFormat = rawFormat;
				PaletteColumnCount = paletteColumnCount;
			});
		}

		private void RefreshTab()
		{
			if(_refreshPending) {
				return;
			}

			_refreshPending = true;
			Dispatcher.UIThread.Post(() => {
				InternalRefreshTab();
				_refreshPending = false;
			});
		}

		private void InternalRefreshTab()
		{
			if(Disposed) {
				return;
			}

			InitBitmap();
				
			lock(_updateLock) {
				Array.Resize(ref _sourceData, _coreSourceData.Length);
				Array.Copy(_coreSourceData, _sourceData, _coreSourceData.Length);
			}

			using(var framebuffer = ViewerBitmap.Lock()) {
				DebugApi.GetTileView(CpuType, GetOptions(), _sourceData, _sourceData.Length, PaletteColors, framebuffer.FrameBuffer.Address);
			}

			if(IsNesChrModeEnabled) {
				DrawNesChrPageDelimiters();
			} else {
				PageDelimiters = null;
			}

			UpdatePreviewPanel();
			LoadSelectedPreset(true);
		}

		private int GetTileAddress(PixelPoint pixelPosition)
		{
			PixelSize tileSize = Config.Format.GetTileSize();
			int bytesPerTile = Config.Format.GetBytesPerTile();
			PixelPoint pos = FromLayoutCoordinates(Config.Layout, new PixelPoint(pixelPosition.X / tileSize.Width, pixelPosition.Y / tileSize.Height));
			int offset = (pos.Y * ColumnCount * 8 / tileSize.Width + pos.X) * bytesPerTile;
			return (Config.StartAddress + offset) % (MaximumAddress + 1);
		}

		private int GetSelectedTileAddress()
		{
			PixelPoint p;
			if(ViewerMousePos.HasValue) {
				p = ViewerMousePos.Value;
			} else {
				if(SelectionRect == default) {
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

			PixelSize tileSize = Config.Format.GetTileSize();
			PixelRect cropRect = new PixelRect(p.X / tileSize.Width * tileSize.Width, p.Y / tileSize.Height * tileSize.Height, tileSize.Width, tileSize.Height);
			entries.AddPicture("Tile", ViewerBitmap, 6, cropRect);

			int address = GetTileAddress(cropRect.TopLeft);
			if(Config.Source.IsRelativeMemory()) {
				entries.AddEntry("Tile address (" + Config.Source.GetShortName() + ")", FormatAddress(address, Config.Source));

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = Config.Source });
				if(absAddress.Address >= 0) {
					entries.AddEntry("Tile address (" + absAddress.Type.GetShortName() + ")", FormatAddress(absAddress.Address, absAddress.Type));
				}
			} else {
				entries.AddEntry("Tile address", FormatAddress(address, Config.Source));
			}

			if(ShowNesTileIndex) {
				entries.AddEntry("Tile index", "$" + ((address >> 4) & 0xFF).ToString("X2"));
			}

			entries.EndUpdate();

			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}

		private string FormatAddress(int address, MemoryType memType)
		{
			if(memType.IsWordAddressing()) {
				return $"${address / 2:X4}.w";
			} else {
				return $"${address:X4}";
			}
		}

		private ContextMenuAction GetEditTileAction(int columnCount, int rowCount, Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = $"{columnCount}x{rowCount} ({GridSizeX * columnCount}px x {GridSizeY * rowCount}px)",
				IsEnabled = () => GetSelectedTileAddress() >= 0,
				OnClick = () => EditTileGrid(columnCount, rowCount, wnd)
			};
		}

		private void EditTileGrid(int columnCount, int rowCount, Window wnd)
		{
			PixelPoint p = ViewerMousePos ?? PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
			List<AddressInfo> addresses = new();
			for(int row = 0; row < rowCount; row++) {
				for(int col = 0; col < columnCount; col++) {
					addresses.Add(new AddressInfo() { Address = GetTileAddress(new PixelPoint(p.X + col*GridSizeX, p.Y + row*GridSizeY)), Type = Config.Source });
				}
			}
			TileEditorWindow.OpenAtTile(
				addresses,
				columnCount,
				Config.Format,
				SelectedPalette,
				wnd,
				CpuType,
				RefreshTiming.Config.RefreshScanline,
				RefreshTiming.Config.RefreshCycle
			);
		}

		private void DrawNesChrPageDelimiters()
		{
			double pageHeight = ((double)256 / ColumnCount) * 8;
			double y = pageHeight;
			List<PictureViewerLine> delimiters = new List<PictureViewerLine>();
			int yMax = RowCount * 8;

			while(y < yMax) {
				//Hide delimiter if the selected tile is right above or below it
				if(Math.Abs(SelectionRect.Top - y) >= 5 && Math.Abs(SelectionRect.Bottom - y) >= 5) {
					Point start = new Point(0, y);
					Point end = new Point(ColumnCount * 8 - 1, y);
					delimiters.Add(new PictureViewerLine() { Start = start, End = end, Color = Colors.Black });
					delimiters.Add(new PictureViewerLine() { Start = start, End = end, Color = Colors.White, DashStyle = new DashStyle(DashStyle.Dash.Dashes, 0) });
				}
				y += pageHeight;
			}
			PageDelimiters = delimiters;
		}

		private bool ShowNesTileIndex
		{
			get { return Config.Source.IsPpuMemory() && Config.Source.ToCpuType() == CpuType.Nes && (Config.StartAddress & 0xFFF) == 0; }
		}

		private bool IsNesChrModeEnabled
		{
			get
			{
				if(ShowNesTileIndex) {
					double rowsPerPage = (double)256 / ColumnCount;
					return rowsPerPage == Math.Floor(rowsPerPage);
				}
				return false;
			}
		}

		private GetTileViewOptions GetOptions()
		{
			return new GetTileViewOptions() {
				MemType = Config.Source,
				Format = Config.Format,
				Width = ColumnCount,
				Height = RowCount,
				Palette = SelectedPalette,
				Layout = Config.Layout,
				Filter = ShowFilterDropdown ? Config.Filter : TileFilter.None,
				StartAddress = Config.StartAddress,
				Background = Config.Background,
				UseGrayscalePalette = Config.UseGrayscalePalette
			};
		}

		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap()
		{
			int width = ColumnCount * 8;
			int height = RowCount * 8;
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		public void OnGameLoaded()
		{
			Dispatcher.UIThread.Post(() => {
				InitForCpuType();
				RefreshData();
			});
		}

		private void ApplyPresetValues(PresetValues preset, bool keepUserConfig)
		{
			Config.Source = preset.Source ?? Config.Source;
			Config.Format = preset.Format ?? Config.Format;
			Config.Filter = preset.Filter ?? Config.Filter;
			Config.RowCount = preset.RowCount ?? Config.RowCount;
			Config.ColumnCount = preset.ColumnCount ?? Config.ColumnCount;
			Config.StartAddress = preset.StartAddress ?? Config.StartAddress;
			if(!keepUserConfig) {
				SelectedPalette = preset.SelectedPalette ?? SelectedPalette;
				Config.Background = preset.Background ?? Config.Background;
				Config.Layout = preset.Layout ?? Config.Layout;
			}
		}

		private void LoadSelectedPreset(bool keepUserConfig)
		{
			if(Config.SelectedPreset != "") {
				ConfigPreset? preset = ConfigPresets.Find(x => x.Name == Config.SelectedPreset);
				if(preset != null) {
					PresetValues? values = preset.GetPresetValues();
					if(values != null) {
						ApplyPresetValues(values, keepUserConfig);
						Config.SelectedPreset = preset.Name;
						preset.Selected = true;
					}
				} else {
					Config.SelectedPreset = "";
				}
			}
		}

		private void ClearPresetSelection()
		{
			if(Config.SelectedPreset != "") {
				ConfigPreset? preset = ConfigPresets.Find(x => x.Name == Config.SelectedPreset);
				if(preset != null) {
					preset.Selected = false;
				}
				Config.SelectedPreset = "";
			}
		}

		private ConfigPreset CreatePreset(int row, string name, Func<PresetValues?> getPresetValues)
		{
			ConfigPreset preset = new ConfigPreset(name, getPresetValues, () => {
				PresetValues? preset = getPresetValues();
				if(preset == null) {
					return;
				}

				ApplyPresetValues(preset, false);

				if(Config.SelectedPreset != name) {
					ClearPresetSelection();
					ConfigPreset? cfgPeset = ConfigPresets.Find(x => x.Name == name);
					if(cfgPeset != null) {
						Config.SelectedPreset = name;
						cfgPeset.Selected = true;
					}
				}
			});

			ConfigPresetRows[row].Add(preset);
			return preset;
		}

		private List<ConfigPreset> GetConfigPresets()
		{
			ConfigPresetRows = new() { new(), new(), new() };

			switch(CpuType) {
				case CpuType.Snes:
					return new() {
						CreatePreset(0, "PPU", () => ApplyPpuPreset()),
						CreatePreset(0, "ROM", () => ApplyPrgPreset()),
						CreatePreset(1, "BG1", () => ApplyBgPreset(0)),
						CreatePreset(1, "BG2", () => ApplyBgPreset(1)),
						CreatePreset(1, "BG3", () => ApplyBgPreset(2)),
						CreatePreset(1, "BG4", () => ApplyBgPreset(3)),
						CreatePreset(2, "OAM1", () => ApplySpritePreset(0)),
						CreatePreset(2, "OAM2", () => ApplySpritePreset(1)),
					};

				case CpuType.Nes:
					return new() {
						CreatePreset(0, "PPU", () => ApplyPpuPreset()),
						CreatePreset(0, "CHR", () => ApplyChrPreset()),
						CreatePreset(0, "ROM", () => ApplyPrgPreset()),
						CreatePreset(1, "BG", () => ApplyBgPreset(0)),
						CreatePreset(1, "OAM", () => ApplySpritePreset(0)),
					};

				case CpuType.Gameboy:
					if(DebugApi.GetPpuState<GbPpuState>(CpuType.Gameboy).CgbEnabled) {
						return new() {
							CreatePreset(0, "PPU", () => ApplyPpuPreset()),
							CreatePreset(0, "ROM", () => ApplyPrgPreset()),
							CreatePreset(1, "BG1", () => ApplyBgPreset(0)),
							CreatePreset(1, "BG2", () => ApplyBgPreset(1)),
							CreatePreset(2, "OAM1", () => ApplySpritePreset(0)),
							CreatePreset(2, "OAM2", () => ApplySpritePreset(1)),
						};
					} else {
						return new() {
							CreatePreset(0, "PPU", () => ApplyPpuPreset()),
							CreatePreset(0, "ROM", () => ApplyPrgPreset()),
							CreatePreset(1, "BG", () => ApplyBgPreset(0)),
							CreatePreset(1, "OAM", () => ApplySpritePreset(0)),
						};
					}

				case CpuType.Pce:
					if(DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine).IsSuperGrafx) {
						return new() {
							CreatePreset(0, "BG1", () => ApplyBgPreset(0)),
							CreatePreset(0, "SPR1", () => ApplySpritePreset(0)),
							CreatePreset(1, "BG2", () => ApplyBgPreset(1)),
							CreatePreset(1, "SPR2", () => ApplySpritePreset(1)),
							CreatePreset(2, "ROM", () => ApplyPrgPreset()),
						};
					} else {
						return new() {
							CreatePreset(0, "BG", () => ApplyBgPreset(0)),
							CreatePreset(0, "Sprites", () => ApplySpritePreset(0)),
							CreatePreset(0, "ROM", () => ApplyPrgPreset()),
						};
					}
				
				case CpuType.Sms:
					return new() {
						CreatePreset(0, "VDP", () => ApplyPpuPreset()),
						CreatePreset(0, "ROM", () => ApplyPrgPreset())
					};

				case CpuType.Gba:
					return new() {
						CreatePreset(0, "VRAM", () => ApplyPpuPreset()),
						CreatePreset(0, "ROM", () => ApplyPrgPreset()),
						CreatePreset(1, "BG1", () => ApplyBgPreset(0)),
						CreatePreset(1, "BG2", () => ApplyBgPreset(1)),
						CreatePreset(1, "BG3", () => ApplyBgPreset(2)),
						CreatePreset(1, "BG4", () => ApplyBgPreset(3)),
						CreatePreset(2, "Sprites", () => ApplySpritePreset(0)),
					};

				case CpuType.Ws:
					return new() {
						CreatePreset(0, "PPU", () => ApplyPpuPreset()),
						CreatePreset(0, "ROM", () => ApplyPrgPreset()),
						CreatePreset(1, "Bank 0", () => ApplyBgPreset(0)),
						CreatePreset(1, "Bank 1", () => ApplyBgPreset(1)),
					};

				default:
					throw new Exception("Unsupported CPU type");
			}
		}

		private PresetValues? ApplyPrgPreset()
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return null;
			}

			PresetValues preset = new();
			preset.Source = CpuType.GetPrgRomMemoryType();
			preset.StartAddress = 0;
			preset.ColumnCount = 16;
			preset.RowCount = 32;
			preset.Layout = TileLayout.Normal;

			if(CpuType == CpuType.Ws) {
				WsPpuState ppu = (WsPpuState)state;
				preset.Format = ppu.Mode.ToTileFormat();
			} else {
				preset.Format = (TileFormat)AvailableFormats[0];
			}

			return preset;
		}

		private PresetValues? ApplyChrPreset()
		{
			PresetValues preset = new();
			preset.Source = DebugApi.GetMemorySize(MemoryType.NesChrRam) > 0 ? MemoryType.NesChrRam : MemoryType.NesChrRom;
			preset.StartAddress = 0;
			preset.ColumnCount = 16;
			preset.RowCount = 32;
			preset.Layout = TileLayout.Normal;
			preset.Format = TileFormat.NesBpp2;
			return preset;
		}

		private PresetValues? ApplyPpuPreset()
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return null;
			}

			PresetValues preset = new();

			switch(CpuType) {
				case CpuType.Snes: {
					preset.Source = MemoryType.SnesVideoRam;
					preset.StartAddress = 0;
					preset.ColumnCount = 16;
					preset.RowCount = 128;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.Bpp4;
					break;
				}

				case CpuType.Nes: {
					preset.Source = MemoryType.NesPpuMemory;
					preset.StartAddress = 0;
					preset.ColumnCount = 16;
					preset.RowCount = 32;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.NesBpp2;
					break;
				}

				case CpuType.Gameboy: {
					preset.Source = MemoryType.GbVideoRam;
					preset.StartAddress = 0;
					preset.ColumnCount = 16;
					preset.RowCount = 32;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.Bpp2;
					break;
				}

				case CpuType.Sms: {
					SmsVdpState vdp = (SmsVdpState)state;
					preset.Source = MemoryType.SmsVideoRam;
					preset.StartAddress = 0;
					preset.ColumnCount = vdp.UseMode4 ? 16 : 32;
					preset.RowCount = vdp.UseMode4 ? 32 : 64;
					preset.Layout = TileLayout.Normal;
					preset.Format = vdp.UseMode4 ? TileFormat.SmsBpp4 : TileFormat.SmsSgBpp1;
					if(!vdp.UseMode4) {
						preset.SelectedPalette = 1;
					}
					break;
				}

				case CpuType.Gba: {
					preset.Source = MemoryType.GbaVideoRam;
					preset.StartAddress = 0;
					preset.ColumnCount = 16;
					preset.RowCount = 128;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.GbaBpp4;
					break;
				}

				case CpuType.Ws: {
					WsPpuState ppu = (WsPpuState)state;
					preset.Source = MemoryType.WsWorkRam;
					preset.StartAddress = 0;
					preset.ColumnCount = 16;
					preset.RowCount = 128;
					preset.Layout = TileLayout.Normal;
					preset.Format = ppu.Mode.ToTileFormat();
					break;
				}
			}
			return preset;
		}

		private PresetValues? ApplyBgPreset(int layer)
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return null;
			}

			PresetValues preset = new();

			switch(CpuType) {
				case CpuType.Snes: {
					int[,] layerBpp = new int[8, 4] { { 2, 2, 2, 2 }, { 4, 4, 2, 0 }, { 4, 4, 0, 0 }, { 8, 4, 0, 0 }, { 8, 2, 0, 0 }, { 4, 2, 0, 0 }, { 4, 0, 0, 0 }, { 8, 0, 0, 0 } };
					SnesPpuState ppu = (SnesPpuState)state;
					preset.Source = MemoryType.SnesVideoRam;
					preset.ColumnCount = 16;
					preset.RowCount = 64;
					preset.Layout = TileLayout.Normal;
					if(ppu.BgMode == 7) {
						preset.Format = ppu.ExtBgEnabled ? TileFormat.Mode7ExtBg : (ppu.DirectColorMode ? TileFormat.Mode7DirectColor : TileFormat.Mode7);
						preset.StartAddress = 0;
						preset.SelectedPalette = 0;
					} else {
						preset.StartAddress = ppu.Layers[layer].ChrAddress * 2;
						preset.Format = layerBpp[ppu.BgMode, layer] switch {
							2 => TileFormat.Bpp2,
							4 => TileFormat.Bpp4,
							8 => ppu.DirectColorMode ? TileFormat.DirectColor : TileFormat.Bpp8,
							_ => TileFormat.Bpp2
						};

						if(layerBpp[ppu.BgMode, layer] == 8 || SelectedPalette >= (layerBpp[ppu.BgMode, layer] == 2 ? 32 : 8)) {
							preset.SelectedPalette = 0;
						}
					}
					break;
				}

				case CpuType.Nes: {
					NesPpuState ppu = (NesPpuState)state;
					preset.Source = MemoryType.NesPpuMemory;
					preset.StartAddress = ppu.Control.BackgroundPatternAddr;
					preset.ColumnCount = 16;
					preset.RowCount = 16;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.NesBpp2;
					if(SelectedPalette >= 4) {
						preset.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = (GbPpuState)state;
					preset.Source = MemoryType.GbVideoRam;
					preset.StartAddress = (layer == 0 ? 0 : 0x2000) | (ppu.BgTileSelect ? 0 : 0x800);
					preset.ColumnCount = 16;
					preset.RowCount = 16;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.Bpp2;
					preset.Background = ppu.CgbEnabled ? TileBackground.PaletteColor : TileBackground.Default;
					if(!ppu.CgbEnabled || SelectedPalette > 8) {
						preset.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Pce: {
					preset.Source = layer == 0 ? MemoryType.PceVideoRam : MemoryType.PceVideoRamVdc2;
					preset.StartAddress = 0;
					preset.ColumnCount = 32;
					preset.RowCount = 64;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.Bpp4;
					preset.Background = TileBackground.Default;
					if(SelectedPalette >= 16) {
						preset.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Gba: {
					GbaPpuState ppu = (GbaPpuState)state;
					preset.Source = MemoryType.GbaVideoRam;
					preset.ColumnCount = 16;
					preset.RowCount = 64;
					preset.Layout = TileLayout.Normal;
					preset.StartAddress = ppu.BgLayers[layer].TilesetAddr;
					preset.Format = ppu.BgLayers[layer].Bpp8Mode ? TileFormat.GbaBpp8 : TileFormat.GbaBpp4;
					if(preset.Format == TileFormat.GbaBpp8 || preset.Format == TileFormat.GbaBpp4 && preset.SelectedPalette > 16) {
						preset.SelectedPalette = 0;
					}
					break;
				}

				case CpuType.Ws: {
					WsPpuState ppu = (WsPpuState)state;
					int bank0Addr = ppu.Mode >= WsVideoMode.Color4bpp ? 0x4000 : 0x2000;
					int bank1Addr = ppu.Mode >= WsVideoMode.Color4bpp ? 0x8000 : (ppu.Mode == WsVideoMode.Monochrome ? 0x2000 : 0x4000);
					
					preset.Source = MemoryType.WsWorkRam;
					preset.StartAddress = layer == 0 ? bank0Addr : bank1Addr;
					preset.ColumnCount = 16;
					preset.RowCount = 128;
					preset.Layout = TileLayout.Normal;
					preset.Format = ppu.Mode.ToTileFormat();
					break;
				}
			}

			return preset;
		}

		private PresetValues? ApplySpritePreset(int layer)
		{
			BaseState? state = _ppuState;
			if(state == null) {
				return null;
			}

			PresetValues preset = new();

			switch(CpuType) {
				case CpuType.Snes: {
					SnesPpuState ppu = (SnesPpuState)state;
					preset.Source = MemoryType.SnesVideoRam;
					preset.Format = TileFormat.Bpp4;
					preset.ColumnCount = 16;
					preset.RowCount = 16;
					preset.StartAddress = (ppu.OamBaseAddress + (layer == 1 ? ppu.OamAddressOffset : 0)) * 2;
					if(SelectedPalette < 8 || SelectedPalette >= 16) {
						preset.SelectedPalette = 8;
					}
					break;
				}

				case CpuType.Nes: {
					NesPpuState ppu = (NesPpuState)state;
					if(ppu.Control.LargeSprites) {
						preset.StartAddress = 0;
						preset.ColumnCount = 16;
						preset.RowCount = 32;
						preset.Layout = TileLayout.SingleLine8x16;
					} else {
						preset.StartAddress = ppu.Control.SpritePatternAddr;
						preset.ColumnCount = 16;
						preset.RowCount = 16;
						preset.Layout = TileLayout.Normal;
					}
					preset.Source = MemoryType.NesPpuMemory;
					if(SelectedPalette < 4 || SelectedPalette >= 8) {
						preset.SelectedPalette = 4;
					}
					preset.Format = TileFormat.NesBpp2;
					break;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = (GbPpuState)state;
					preset.Source = MemoryType.GbVideoRam;
					preset.StartAddress = layer == 0 ? 0 : 0x2000;
					preset.ColumnCount = 16;
					preset.RowCount = 16;
					preset.Layout = TileLayout.Normal;
					preset.Background = TileBackground.Black;
					preset.Format = TileFormat.Bpp2;
					if(ppu.CgbEnabled && SelectedPalette < 8) {
						preset.SelectedPalette = 8;
					} else if(!ppu.CgbEnabled && SelectedPalette == 0) {
						preset.SelectedPalette = 1;
					}
					break;
				}

				case CpuType.Pce: {
					preset.Source = layer == 0 ? MemoryType.PceVideoRam : MemoryType.PceVideoRamVdc2;
					preset.StartAddress = 0;
					preset.ColumnCount = 32;
					preset.RowCount = 64;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.PceSpriteBpp4;
					preset.Background = TileBackground.Default;
					if(SelectedPalette < 16) {
						preset.SelectedPalette = 16;
					}
					break;
				}

				case CpuType.Gba: {
					preset.Source = MemoryType.GbaVideoRam;
					preset.StartAddress = 0x10000;
					preset.ColumnCount = 32;
					preset.RowCount = 32;
					preset.Layout = TileLayout.Normal;
					preset.Format = TileFormat.GbaBpp4;
					preset.Background = TileBackground.Default;
					if(SelectedPalette < 16) {
						preset.SelectedPalette = 16;
					}
					break;
				}
			}
			return preset;
		}
	}

	public class ConfigPreset : ViewModelBase
	{
		public string Name { get; }
		public Func<PresetValues?> GetPresetValues { get; }
		public ReactiveCommand<Unit, Unit> ClickCommand { get; }
		public Action ApplyPreset { get; }

		[Reactive] public bool Selected { get; set; }

		public ConfigPreset(string name, Func<PresetValues?> getPresetValues, Action applyPreset)
		{
			Name = name;
			GetPresetValues = getPresetValues;
			ApplyPreset = applyPreset;
			ClickCommand = ReactiveCommand.Create(ApplyPreset);
		}
	}

	public class PresetValues
	{
		public MemoryType? Source { get; set; }
		public TileFormat? Format { get; set; }
		public TileLayout? Layout { get; set; }
		public TileFilter? Filter { get; set; }
		public TileBackground? Background { get; set; }
		public int? RowCount { get; set; }
		public int? ColumnCount { get; set; }
		public int? StartAddress { get; set; }
		public int? SelectedPalette { get; set; }
	}
}
