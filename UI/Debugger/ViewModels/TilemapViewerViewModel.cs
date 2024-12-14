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

		[Reactive] public List<GridDefinition>? CustomGrids { get; set; } = null;

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public DynamicTooltip TilemapInfoPanel { get; private set; } = new DynamicTooltip();
		[Reactive] public bool IsTilemapInfoVisible { get; private set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }
		[Reactive] public DynamicTooltip? ViewerTooltip { get; set; }
		[Reactive] public PixelPoint? ViewerMousePos { get; set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; private set; } = new List<TilemapViewerTab>();
		[Reactive] public bool ShowTabs { get; private set; }
		[Reactive] public TilemapViewerTab SelectedTab { get; set; }

		[Reactive] public Rect ScrollOverlayRect { get; private set; }
		[Reactive] public List<PictureViewerLine>? OverlayLines { get; private set; } = null;

		[Reactive] public Enum[] AvailableDisplayModes { get; set; } = Array.Empty<Enum>();

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private object _updateLock = new();
		private TilemapViewerData _data = new();
		private TilemapViewerData _coreData = new();

		private PictureViewer _picViewer;
		private bool _refreshDataOnTabChange;
		private bool _inGameLoaded;
		private bool _refreshPending;

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
					ActionType = ActionType.ExportToPng,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveAsPng),
					OnClick = () => _picViewer.ExportToPng()
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

			AddDisposables(DebugShortcutManager.CreateContextMenu(picViewer, new List<object>() {
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
							TileViewerWindow.OpenAtTile(CpuType, GetVramMemoryType(), tile.Value.TileAddress, _data.TilemapInfo.Format, TileLayout.Normal, tile.Value.PaletteIndex);
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
						GetEditTileAction(1, 2, wnd),
						GetEditTileAction(2, 1, wnd),
						GetEditTileAction(2, 2, wnd),
						GetEditTileAction(4, 4, wnd),
						GetEditTileAction(8, 8, wnd)
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
				new ContextMenuSeparator() { IsVisible = () => CpuType == CpuType.Nes },
				new ContextMenuAction() {
					ActionType = ActionType.CopyToHdPackFormat,
					IsVisible = () => CpuType == CpuType.Nes,
					IsEnabled = () => HdPackCopyHelper.IsActionAllowed(GetVramMemoryType()),
					OnClick = () => {
						DebugTilemapTileInfo? tile = GetSelectedTileInfo();
						if(tile != null && tile?.TileAddress >= 0) {
							HdPackCopyHelper.CopyToHdPackFormat(tile.Value.TileAddress, GetVramMemoryType(), _data.RawPalette, tile.Value.PaletteIndex, false);
						}
					}
				}
			}));

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
			
			InitNesGridOptions();

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		private void InitNesGridOptions()
		{
			AddDisposable(this.WhenAnyValue(x => x.Config.NesShowAttributeGrid, x => x.Config.NesShowAttributeByteGrid, x => x.Config.NesShowTilemapGrid, x => x.CpuType).Subscribe(x => {
				if(CpuType == CpuType.Nes) {
					List<GridDefinition> grids = new();
					if(Config.NesShowAttributeGrid) {
						grids.Add(new() { SizeX = 16, SizeY = 16, Color = Colors.Red });
					}
					if(Config.NesShowAttributeByteGrid) {
						grids.Add(new() { SizeX = 32, SizeY = 32, Color = Colors.LightGreen, RestartY = 240 });
					}
					if(Config.NesShowTilemapGrid) {
						grids.Add(new() { SizeX = 256, SizeY = 240, Color = Colors.LightGray });
					}
					CustomGrids = grids;
				} else {
					CustomGrids = null;
				}
			}));
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

			if(IsNes) {
				AvailableDisplayModes = new Enum[] { TilemapDisplayMode.Default, TilemapDisplayMode.Grayscale, TilemapDisplayMode.AttributeView };
			} else {
				AvailableDisplayModes = new Enum[] { TilemapDisplayMode.Default, TilemapDisplayMode.Grayscale };
			}

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
					FrameInfo size = DebugApi.GetTilemapSize(CpuType.Nes, new GetTilemapOptions() { Layer = 1 }, new NesPpuState());
					if(size.Width != 0 && size.Height != 0) {
						Tabs = new List<TilemapViewerTab>() {
							new() { Title = "Nametables", Layer = 0 },
							new() { Title = "Window", Layer = 1 }
						};
					} else {
						Tabs = new List<TilemapViewerTab>() {
							new() { Title = "", Layer = 0 }
						};
					}
					break;

				case CpuType.Sms:
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

				case CpuType.Gba: {
					Tabs = new() {
						new() { Title = "BG0", Layer = 0 },
						new() { Title = "BG1", Layer = 1 },
						new() { Title = "BG2", Layer = 2 },
						new() { Title = "BG3", Layer = 3 },
						new() { Title = "Mem Access", Layer = 4 }
					};
					break;
				}

				case CpuType.Ws:
					Tabs = new() {
						new() { Title = "BG0", Layer = 0 },
						new() { Title = "BG1", Layer = 1 }
					};
					break;

				default:
					throw new Exception("unsupported cpu type");
			}
		}

		private DebugTilemapTileInfo? GetSelectedTileInfo()
		{
			if(_data.PpuState == null || _data.PpuToolsState == null || _data.Vram == null) {
				return null;
			} else {
				PixelPoint p;
				if(ViewerMousePos.HasValue) {
					p = ViewerMousePos.Value;
				} else {
					if(SelectionRect == default) {
						return null;
					}
					p = PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
				}
				return DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _data.Vram, _data.PpuState, _data.PpuToolsState);
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
			lock(_updateLock) {
				_coreData.MasterClock = EmuApi.GetTimingInfo(CpuType).MasterClock;

				BaseState ppuState = DebugApi.GetPpuState(CpuType);
				_coreData.PpuState = ppuState;
				_coreData.PpuToolsState = DebugApi.GetPpuToolsState(CpuType);
				_coreData.PrevVram = _coreData.Vram;
				DebugApi.GetMemoryState(GetVramMemoryType(), ref _coreData.Vram);
				DebugApi.GetMemoryAccessCounts(GetVramMemoryType(), ref _coreData.AccessCounters);

				DebugPaletteInfo palette = DebugApi.GetPaletteInfo(CpuType);
				_coreData.RgbPalette = palette.GetRgbPalette();
				_coreData.RawPalette = palette.GetRawPalette();
				_coreData.RawFormat = palette.RawFormat;
			}

			RefreshTab();
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

			lock(_updateLock) {
				_coreData.CopyTo(_data);
			}

			if(_data.PpuState == null || _data.PpuToolsState == null) {
				_refreshPending = false;
				return;
			}

			GetTilemapOptions options;
			FrameInfo size;

			foreach(TilemapViewerTab tab in Tabs) {
				options = GetOptions(tab);
				size = DebugApi.GetTilemapSize(CpuType, options, _data.PpuState);
				tab.Enabled = size.Width != 0 && size.Height != 0;
			}

			if(!SelectedTab.Enabled) {
				foreach(TilemapViewerTab tab in Tabs) {
					if(tab.Enabled) {
						SelectedTab = tab;
						break;
					}
				}
			}

			options = GetOptions(SelectedTab, _data.PrevVram, _data.AccessCounters);
			options.MasterClock = Interlocked.Read(ref _data.MasterClock);

			size = DebugApi.GetTilemapSize(CpuType, options, _data.PpuState);
			InitBitmap((int)size.Width, (int)size.Height);

			using(var framebuffer = ViewerBitmap.Lock()) {
				_data.TilemapInfo = DebugApi.GetTilemap(CpuType, options, _data.PpuState, _data.PpuToolsState, _data.Vram, _data.RgbPalette, framebuffer.FrameBuffer.Address);
			}

			if(_data.TilemapInfo.Bpp == 0) {
				GridSizeX = 8;
				GridSizeY = 8;
				ScrollOverlayRect = default;
				OverlayLines = null;
				PreviewPanel = null;
				IsTilemapInfoVisible = false;
				return;
			}

			IsTilemapInfoVisible = true;

			GridSizeX = (int)_data.TilemapInfo.TileWidth;
			GridSizeY = (int)_data.TilemapInfo.TileHeight;

			UpdatePreviewPanel();
			UpdateTilemapInfo();

			if(Config.ShowScrollOverlay) {
				ScrollOverlayRect = new Rect(
					_data.TilemapInfo.ScrollX % size.Width,
					_data.TilemapInfo.ScrollY % size.Height,
					_data.TilemapInfo.ScrollWidth,
					_data.TilemapInfo.ScrollHeight
				);

				DrawMode7Overlay();
			} else {
				ScrollOverlayRect = default;
				OverlayLines = null;
			}

			_refreshPending = false;
		}

		private void UpdateTilemapInfo()
		{
			TooltipEntries entries = TilemapInfoPanel.Items ?? new TooltipEntries();
			DebugTilemapInfo info = _data.TilemapInfo;
			entries.StartUpdate();
			entries.AddEntry("Size", info.ColumnCount + "x" + info.RowCount);
			entries.AddEntry("Size (px)", info.ColumnCount* info.TileWidth + "x" + info.RowCount* info.TileHeight);
			entries.AddEntry("Tilemap Address", FormatAddress((int)info.TilemapAddress));
			entries.AddEntry("Tileset Address", FormatAddress((int)info.TilesetAddress));
			entries.AddEntry("Tile Format", info.Format);
			if(info.Mirroring != TilemapMirroring.None) {
				entries.AddEntry("Mirroring", info.Mirroring);
			}
			if(info.Priority >= 0) {
				entries.AddEntry("Priority", info.Priority);
			}
			entries.EndUpdate();
			TilemapInfoPanel.Items = entries;
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			if(_data.PpuState == null || _data.PpuToolsState == null) {
				return null;
			}

			DebugTilemapTileInfo? result = DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(SelectedTab), _data.Vram, _data.PpuState, _data.PpuToolsState);
			if(result == null) {
				return null;
			}

			DebugTilemapTileInfo tileInfo = result.Value;

			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			PixelRect cropRect = new PixelRect(p.X / tileInfo.Width * tileInfo.Width, p.Y / tileInfo.Height * tileInfo.Height, tileInfo.Width, tileInfo.Height);

			entries.StartUpdate();

			if(tileInfo.Width == 1 && tileInfo.Height == 1) {
				entries.AddPicture("Tile", ViewerBitmap, 32, cropRect);
			} else {
				entries.AddPicture("Tile", ViewerBitmap, 6, cropRect);
			}

			if(_data.TilemapInfo.Bpp >= 2 && _data.TilemapInfo.Bpp <= 4) {
				int paletteSize = (int)Math.Pow(2, _data.TilemapInfo.Bpp);
				int paletteIndex = tileInfo.PaletteIndex >= 0 ? tileInfo.PaletteIndex : 0;
				entries.AddEntry("Palette", new TooltipPaletteEntry(paletteIndex, paletteSize, _data.RgbPalette, _data.RawPalette, _data.RawFormat));
			}

			if(tileInfo.Width != 1 || tileInfo.Height != 1) {
				entries.AddEntry("Column, Row", $"{tileInfo.Column}, {tileInfo.Row}");
			}
			entries.AddEntry("X, Y", $"{tileInfo.Column*tileInfo.Width}, {tileInfo.Row*tileInfo.Height}");
			entries.AddEntry("Size", tileInfo.Width + "x" + tileInfo.Height);

			if(tileInfo.TileMapAddress >= 0) {
				entries.AddEntry("Tilemap address", FormatAddress(tileInfo.TileMapAddress));
			}

			entries.AddSeparator("TileSeparator");

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

			entries.AddSeparator("PaletteSeparator");

			if(tileInfo.PaletteIndex >= 0) {
				if(tileInfo.BasePaletteIndex >= 0) {
					entries.AddEntry("Palette index", $"{tileInfo.BasePaletteIndex} ({tileInfo.PaletteIndex})");
				} else {
					entries.AddEntry("Palette index", tileInfo.PaletteIndex.ToString());
				}
			}
			if(tileInfo.PaletteAddress >= 0) {
				entries.AddEntry("Palette address", "$" + tileInfo.PaletteAddress.ToString("X2"));
			}

			entries.AddSeparator("AttributeSeparator");

			if(tileInfo.AttributeAddress >= 0) {
				entries.AddEntry("Attribute address", "$" + tileInfo.AttributeAddress.ToString("X4"));
			}
			if(tileInfo.AttributeData >= 0) {
				entries.AddEntry("Attribute data", "$" + tileInfo.AttributeData.ToString("X2"));
			}

			entries.AddSeparator("MiscSeparator");

			if(tileInfo.PixelData >= 0) {
				entries.AddEntry("Pixel data", "$" + tileInfo.PixelData.ToString("X2"));
			}
			entries.AddEntry("Horizontal mirror", tileInfo.HorizontalMirroring);
			entries.AddEntry("Vertical mirror", tileInfo.VerticalMirroring);
			entries.AddEntry("High priority", tileInfo.HighPriority);

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

		private ContextMenuAction GetEditTileAction(int columnCount, int rowCount, Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = $"{columnCount}x{rowCount} ({GridSizeX * columnCount}px x {GridSizeY * rowCount}px)",
				OnClick = () => EditTileGrid(columnCount, rowCount, wnd)
			};
		}

		private void EditTileGrid(int columnCount, int rowCount, Window wnd)
		{
			if(_data.PpuState == null || _data.PpuToolsState == null) {
				return;
			}

			PixelPoint p = ViewerMousePos ?? PixelPoint.FromPoint(SelectionRect.TopLeft, 1);
			List<AddressInfo> addresses = new();
			MemoryType memType = GetVramMemoryType();
			int palette = -1;
			for(int row = 0; row < rowCount; row++) {
				for(int col = 0; col < columnCount; col++) {
					DebugTilemapTileInfo? tile = DebugApi.GetTilemapTileInfo((uint)(p.X + GridSizeX*col), (uint)(p.Y + GridSizeY*row), CpuType, GetOptions(SelectedTab), _data.Vram, _data.PpuState, _data.PpuToolsState);
					if(tile == null) {
						if(col == 0) {
							rowCount = row;
							break;
						} else {
							columnCount = col;
							continue;
						}
					}

					if(palette == -1) {
						palette = tile.Value.PaletteIndex;
					}
					addresses.Add(new AddressInfo() { Address = tile.Value.TileAddress, Type = memType });
				}
			}

			if(rowCount <= 0 || columnCount <= 0) {
				return;
			}

			palette = Math.Max(0, palette);
			TileEditorWindow.OpenAtTile(
				addresses,
				columnCount,
				_data.TilemapInfo.Format,
				palette,
				wnd,
				CpuType,
				RefreshTiming.Config.RefreshScanline,
				RefreshTiming.Config.RefreshCycle
			);
		}

		private void DrawMode7Overlay()
		{
			if(_data.PpuToolsState is SnesPpuToolsState toolsState && _data.PpuState is SnesPpuState ppuState && ppuState.BgMode == 7) {
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
			Dispatcher.UIThread.Post(() => {
				_inGameLoaded = true;
				InitForCpuType();
				RefreshData();
				_inGameLoaded = false;
			});
		}
	}

	public class TilemapViewerTab : ViewModelBase
	{
		[Reactive] public string Title { get; set; } = "";
		[Reactive] public int Layer { get; set; }  = 0;
		[Reactive] public MemoryType? VramMemoryType { get; set; }
		[Reactive] public bool Enabled { get; set; } = true;
	}

	public class TilemapViewerData
	{
		public DebugTilemapInfo TilemapInfo;
		public UInt64 MasterClock;
		public BaseState? PpuState;
		public BaseState? PpuToolsState;
		public byte[] PrevVram = Array.Empty<byte>();
		public byte[] Vram = Array.Empty<byte>();
		public UInt32[] RgbPalette = Array.Empty<UInt32>();
		public UInt32[] RawPalette = Array.Empty<UInt32>();
		public RawPaletteFormat RawFormat;
		public AddressCounters[] AccessCounters = Array.Empty<AddressCounters>();

		public void CopyTo(TilemapViewerData dst)
		{
			dst.TilemapInfo = TilemapInfo;
			dst.MasterClock = MasterClock;
			dst.PpuState = PpuState;
			dst.PpuToolsState = PpuToolsState;
			dst.RawFormat = RawFormat;

			CopyArray(PrevVram, ref dst.PrevVram);
			CopyArray(Vram, ref dst.Vram);
			CopyArray(RgbPalette, ref dst.RgbPalette);
			CopyArray(RawPalette, ref dst.RawPalette);
			CopyArray(AccessCounters, ref dst.AccessCounters);
		}

		private void CopyArray<T>(T[] src, ref T[] dst)
		{
			if(src.Length != dst.Length) {
				Array.Resize(ref dst, src.Length);
			}
			Array.Copy(src, dst, src.Length);
		}
	}
}
