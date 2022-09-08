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
using System.Threading;

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
		[Reactive] public bool IsTilemapInfoVisible { get; private set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }
		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; private set; } = new List<TilemapViewerTab>();
		[Reactive] public bool ShowTabs { get; private set; }
		[Reactive] public TilemapViewerTab SelectedTab { get; set; }

		[Reactive] public Rect ScrollOverlayRect { get; private set; } = Rect.Empty;
		[Reactive] public List<PictureViewerLine>? OverlayLines { get; private set; } = null;
		
		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();
		
		private DebugTilemapInfo _tilemapInfo;
		private PictureViewer _picViewer;
		private UInt64 _masterClock;
		private BaseState? _ppuState;
		private BaseState? _ppuToolsState;
		private byte[] _prevVram = Array.Empty<byte>();
		private byte[] _vram = Array.Empty<byte>();
		private UInt32[] _rgbPalette = Array.Empty<UInt32>();
		private UInt32[] _rawPalette = Array.Empty<UInt32>();
		private RawPaletteFormat _rawFormat;
		private AddressCounters[] _accessCounters = Array.Empty<AddressCounters>();
		private bool _refreshDataOnTabChange;
		private bool _inGameLoaded;

		[Obsolete("For designer only")]
		public TilemapViewerViewModel() : this(CpuType.Snes, new PictureViewer(), null) { }

		public TilemapViewerViewModel(CpuType cpuType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TilemapViewer.Clone();
			CpuType = cpuType;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming, cpuType);
			
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

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			DebugShortcutManager.CreateContextMenu(picViewer, new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.ViewInMemoryViewer,
					HintText = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						return tile?.TileMapAddress > 0 ? $"${tile?.TileMapAddress:X4}" : "";
					},
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
					ActionType = ActionType.EditTile,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_EditTile),
					OnClick = () => EditTileGrid(1, 1, wnd)
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditTiles,
					SubActions = new() {
						new ContextMenuAction() {
							ActionType = ActionType.Custom,
							CustomText = $"1x2 ({GridSizeX}px x {GridSizeY*2}px)",
							OnClick = () => EditTileGrid(1, 2, wnd)
						},
						new ContextMenuAction() {
							ActionType = ActionType.Custom,
							CustomText = $"2x1 ({GridSizeX*2}px x {GridSizeY}px)",
							OnClick = () => EditTileGrid(2, 1, wnd)
						},
						new ContextMenuAction() {
							ActionType = ActionType.Custom,
							CustomText = $"2x2 ({GridSizeX*2}px x {GridSizeY*2}px)",
							OnClick = () => EditTileGrid(2, 2, wnd)
						},
						new ContextMenuAction() {
							ActionType = ActionType.Custom,
							CustomText = $"4x4 ({GridSizeX*4}px x {GridSizeY*4}px)",
							OnClick = () => EditTileGrid(4, 4, wnd)
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.EditTilemapBreakpoint,
					HintText = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						return tile?.TileMapAddress > 0 ? $"${tile?.TileMapAddress:X4}" : "";
					},
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_EditTilemapBreakpoint),
					OnClick = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						if(tile != null && tile.Value.TileMapAddress >= 0) {
							EditBreakpoint(wnd, tile.Value.TileMapAddress);
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.EditAttributeBreakpoint,
					HintText = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						return tile?.AttributeAddress > 0 ? $"${tile?.AttributeAddress:X4}"  : "";
					},
					IsVisible = () => IsNes,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.TilemapViewer_EditAttributeBreakpoint),
					OnClick = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						if(tile != null && tile.Value.AttributeAddress >= 0) {
							EditBreakpoint(wnd, tile.Value.AttributeAddress);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.ExportToPng,
					OnClick = () => _picViewer.ExportToPng()
				}
			});

			AddDisposable(this.WhenAnyValue(x => x.Tabs).Subscribe(x => ShowTabs = x.Count > 1));
			AddDisposable(this.WhenAnyValue(x => x.SelectedTab).Subscribe(x => {
				if(_inGameLoaded) {
					//Skip refresh data/tab if this is triggered while processing a gameloaded event
					//Otherwise RefreshTab will be called on the old game's data, causing a crash.
					return;
				}

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

		private async void EditBreakpoint(Window wnd, int address)
		{
			MemoryType memType = GetVramMemoryType();
			AddressInfo addr = new AddressInfo() { Address = address, Type = memType };
			if(memType.IsRelativeMemory()) {
				AddressInfo absAddr = DebugApi.GetAbsoluteAddress(addr);
				if(absAddr.Address >= 0) {
					addr = absAddr;
				}
			}

			if(addr.Address >= 0) {
				Breakpoint? bp = BreakpointManager.GetMatchingBreakpoint(addr, CpuType);
				if(bp == null) {
					bp = new Breakpoint() {
						BreakOnWrite = true,
						BreakOnRead = true,
						CpuType = CpuType,
						StartAddress = (uint)addr.Address,
						EndAddress = (uint)addr.Address,
						MemoryType = addr.Type
					};
				}

				bool result = await BreakpointEditWindow.EditBreakpointAsync(bp, wnd);
				if(result && DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == CpuType) == null) {
					DebuggerWindow.GetOrOpenWindow(CpuType);
				}
			}
		}

		private void InitForCpuType()
		{
			IsNes = CpuType == CpuType.Nes;
			_refreshDataOnTabChange = false;
			switch(CpuType) {
				case CpuType.Snes:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "Layer 1", Layer = 0 },
						new() { Title = "Layer 2", Layer = 1 },
						new() { Title = "Layer 3", Layer = 2 },
						new() { Title = "Layer 4", Layer = 3 },
						new() { Title = "Main", Layer = 4 },
						new() { Title = "Sub", Layer = 5 },
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
			if(_ppuState == null || _vram == null) {
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
				return DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _vram, _ppuState);
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

		private GetTilemapOptions GetOptions(TilemapViewerTab tab, byte[]? prevVram = null, AddressCounters[]? accessCounters = null)
		{
			return new GetTilemapOptions() {
				Layer = (byte)tab.Layer,
				CompareVram = prevVram?.Length > 0 ? prevVram : null,
				AccessCounters = accessCounters,
				TileHighlightMode = Config.TileHighlightMode,
				AttributeHighlightMode = Config.AttributeHighlightMode,
				DisplayMode = Config.DisplayMode
			};
		}

		private MemoryType GetVramMemoryType()
		{
			return SelectedTab?.VramMemoryType ?? CpuType.GetVramMemoryType();
		}

		public void RefreshData()
		{
			Interlocked.Exchange(ref _masterClock, EmuApi.GetTimingInfo(CpuType).MasterClock);

			BaseState ppuState = DebugApi.GetPpuState(CpuType);
			_ppuState = ppuState;
			_ppuToolsState = DebugApi.GetPpuToolsState(CpuType);
			_prevVram = _vram;
			_vram = DebugApi.GetMemoryState(GetVramMemoryType());
			_accessCounters = DebugApi.GetMemoryAccessCounts(GetVramMemoryType());

			DebugPaletteInfo palette = DebugApi.GetPaletteInfo(CpuType);
			_rgbPalette = palette.GetRgbPalette();
			_rawPalette = palette.GetRawPalette();
			_rawFormat = palette.RawFormat;

			RefreshTab();
		}

		private void RefreshTab()
		{
			Dispatcher.UIThread.Post(() => {
				if(_ppuState == null) {
					return;
				}

				BaseState ppuState = _ppuState;
				byte[] prevVram = _prevVram;
				byte[] vram = _vram;
				uint[] palette = _rgbPalette;
				AddressCounters[] accessCounters = _accessCounters;

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

				options = GetOptions(SelectedTab, prevVram, accessCounters);
				options.MasterClock = Interlocked.Read(ref _masterClock);

				size = DebugApi.GetTilemapSize(CpuType, options, ppuState);
				InitBitmap((int)size.Width, (int)size.Height);

				using(var framebuffer = ViewerBitmap.Lock()) {
					_tilemapInfo = DebugApi.GetTilemap(CpuType, options, ppuState, vram, palette, framebuffer.FrameBuffer.Address);
				}

				if(_tilemapInfo.Bpp == 0) {
					GridSizeX = 8;
					GridSizeY = 8;
					ScrollOverlayRect = Rect.Empty;
					OverlayLines = null;
					PreviewPanel = null;
					IsTilemapInfoVisible = false;
					return;
				}

				IsTilemapInfoVisible = true;

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

					DrawMode7Overlay();
				} else {
					ScrollOverlayRect = Rect.Empty;
					OverlayLines = null;
				}
			});
		}

		private void UpdateTilemapInfo()
		{
			TooltipEntries entries = TilemapInfoPanel.Items ?? new TooltipEntries();
			DebugTilemapInfo info = _tilemapInfo;
			entries.StartUpdate();
			entries.AddEntry("Size", info.ColumnCount + "x" + info.RowCount);
			entries.AddEntry("Size (px)", info.ColumnCount* info.TileWidth + "x" + info.RowCount* info.TileHeight);
			entries.AddEntry("Tilemap Address", FormatAddress((int)info.TilemapAddress));
			entries.AddEntry("Tileset Address", FormatAddress((int)info.TilesetAddress));
			entries.AddEntry("Tile Format", info.Format);
			if(info.Mirroring != TilemapMirroring.None) {
				entries.AddEntry("Mirroring", info.Mirroring);
			}
			entries.EndUpdate();
			TilemapInfoPanel.Items = entries;
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			if(_ppuState == null) {
				return null;
			}

			DebugTilemapTileInfo? result = DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _vram, _ppuState);
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
				entries.AddEntry("Palette", new TooltipPaletteEntry(paletteIndex, paletteSize, _rgbPalette, _rawPalette, _rawFormat));
			}

			entries.AddEntry("Column, Row", $"{tileInfo.Column}, {tileInfo.Row}");
			entries.AddEntry("X, Y", $"{tileInfo.Column*tileInfo.Width}, {tileInfo.Row*tileInfo.Height}");
			entries.AddEntry("Size", tileInfo.Width + "x" + tileInfo.Height);

			if(tileInfo.TileMapAddress >= 0) {
				entries.AddEntry("Tilemap address", FormatAddress(tileInfo.TileMapAddress));
			}
			if(tileInfo.TileIndex >= 0) {
				entries.AddEntry("Tile index", "$" + tileInfo.TileIndex.ToString("X2"));
			}
			if(tileInfo.TileAddress >= 0) {
				MemoryType memType = GetVramMemoryType();
				if(memType.IsRelativeMemory()) {
					entries.AddEntry("Tile address (" + memType.GetShortName() + ")", "$" + tileInfo.TileAddress.ToString("X4"));

					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = tileInfo.TileAddress, Type = memType });
					if(absAddress.Address >= 0) {
						entries.AddEntry("Tile address (" + absAddress.Type.GetShortName() + ")", "$" + absAddress.Address.ToString("X4"));
					}
				} else {
					entries.AddEntry("Tile address", FormatAddress(tileInfo.TileAddress));
				}
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

		private string FormatAddress(int address)
		{
			if(GetVramMemoryType().IsWordAddressing()) {
				return $"${address / 2:X4}.w";
			} else {
				return $"${address:X4}";
			}
		}

		private void EditTileGrid(int columnCount, int rowCount, Window wnd)
		{
			if(_ppuState != null) {
				PixelPoint p = ViewerMousePos ?? PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
				List<AddressInfo> addresses = new();
				MemoryType memType = GetVramMemoryType();
				int palette = -1;
				for(int row = 0; row < rowCount; row++) {
					for(int col = 0; col < columnCount; col++) {
						DebugTilemapTileInfo? tile = DebugApi.GetTilemapTileInfo((uint)(p.X + GridSizeX*col), (uint)(p.Y + GridSizeY*row), CpuType, GetOptions(SelectedTab), _vram, _ppuState);
						if(tile == null) {
							return;
						}

						if(palette == -1) {
							palette = tile.Value.PaletteIndex;
						}
						addresses.Add(new AddressInfo() { Address = tile.Value.TileAddress, Type = memType });
					}
				}
				palette = Math.Max(0, palette);
				TileEditorWindow.OpenAtTile(addresses, columnCount, _tilemapInfo.Format, palette, wnd);
			}
		}

		private void DrawMode7Overlay()
		{
			if(_ppuToolsState is SnesPpuToolsState toolsState && _ppuState is SnesPpuState ppuState) {
				List<PictureViewerLine> lines = new();

				Point prevStart = new();
				Point prevEnd = new();
				void AddLine(Point start, Point end, Color color) {
					if(start != prevStart && end != prevEnd) {
						lines.Add(new PictureViewerLine() { Start = start, End = end, Width = 1.5, Color = color });
						prevStart = start;
						prevEnd = end;
					}
				}

				Mesen.Utilities.HslColor baseColor = ColorHelper.RgbToHsl(Color.FromRgb(255, 0, 255));
				for(int i = 0; i < 239; i++) {
					if(toolsState.ScanlineBgMode[i] == 7) {
						Color lineColor = ColorHelper.HslToRgb(baseColor);
						Color alphaColor = Color.FromArgb(0xA0, lineColor.R, lineColor.G, lineColor.B);

						int startX = toolsState.Mode7StartX[i] >> 8;
						int startY = toolsState.Mode7StartY[i] >> 8;
						int endX = toolsState.Mode7EndX[i] >> 8;
						int endY = toolsState.Mode7EndY[i] >> 8;

						AddLine(new Point(startX, startY), new Point(endX, endY), alphaColor);
						if(!ppuState.Mode7.LargeMap) {
							void Translate(ref int start, ref int end, int offset, Func<int, bool> predicate) {
								while(predicate(start) || predicate(end)) {
									start += offset;
									end += offset;
									AddLine(new Point(startX, startY), new Point(endX, endY), alphaColor);
								}
							}

							Translate(ref startX, ref endX, 1024, x => x < 0);
							Translate(ref startY, ref endY, 1024, x => x < 0);
							Translate(ref startX, ref endX, -1024, x => x >= 1024);
							Translate(ref startY, ref endY, -1024, x => x >= 1024);
						}
					}
					baseColor.H = (baseColor.H + 1) % 360;
				}
				OverlayLines = lines;
			} else {
				OverlayLines = null;
			}
		}

		public void OnGameLoaded()
		{
			_inGameLoaded = true;
			InitForCpuType();
			RefreshData();
			_inGameLoaded = false;
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
