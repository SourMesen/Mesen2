using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
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
	public class TilemapViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		public TilemapViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }

		[Reactive] public Rect SelectionRect { get; set; }

		[Reactive] public DynamicBitmap ViewerBitmap { get; private set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; private set; } = new List<TilemapViewerTab>();
		[Reactive] public bool ShowTabs { get; private set; }
		[Reactive] public TilemapViewerTab SelectedTab { get; set; }

		[Reactive] public Rect ScrollOverlayRect { get; private set; } = Rect.Empty;

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();

		private PictureViewer _picViewer;
		private BaseState? _ppuState;
		private byte[]? _prevVram;
		private byte[] _vram = Array.Empty<byte>();
		private UInt32[] _palette = Array.Empty<UInt32>();

		[Obsolete("For designer only")]
		public TilemapViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes, new PictureViewer(), null) { }

		public TilemapViewerViewModel(CpuType cpuType, ConsoleType consoleType, PictureViewer picViewer, Window? wnd)
		{
			Config = ConfigManager.Config.Debug.TilemapViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);

			CpuType = cpuType;
			ConsoleType = consoleType;

			_picViewer = picViewer;

			switch(CpuType) {
				case CpuType.Cpu: 
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

				case CpuType.Gameboy:
					Tabs = new List<TilemapViewerTab>() {
						new() { Title = "Layer 1", Layer = 0 },
						new() { Title = "Layer 2", Layer = 1 }
					};
					break;
			}

			SelectedTab = Tabs[0];

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
					OnClick = () => _picViewer.ZoomIn()
				},
				new ContextMenuAction() {
					ActionType = ActionType.ZoomOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ZoomOut),
					OnClick = () => _picViewer.ZoomOut()
				},
			};

			if(Design.IsDesignMode || wnd == null) {
				return;
			}

			this.WhenAnyValue(x => x.Tabs).Subscribe(x => ShowTabs = x.Count > 1);

			this.WhenAnyValue(x => x.SelectedTab).Subscribe(x => RefreshTab());

			this.WhenAnyValue(x => x.SelectionRect).Subscribe(x => UpdatePreviewPanel());

			Config.PropertyChanged += Config_PropertyChanged;

			DebugShortcutManager.RegisterActions(wnd, FileMenuActions);
			DebugShortcutManager.RegisterActions(wnd, ViewMenuActions);
		}

		private void UpdatePreviewPanel()
		{
			if(SelectionRect.IsEmpty) {
				PreviewPanel = null;
			} else {
				PreviewPanel = GetPreviewPanel(PixelPoint.FromPoint(SelectionRect.TopLeft, 1), PreviewPanel);
			}
		}

		private void Config_PropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			RefreshTab();
		}

		public void Dispose()
		{
			Config.PropertyChanged -= Config_PropertyChanged;
		}
		
		[MemberNotNull(nameof(ViewerBitmap))]
		private void InitBitmap(int width, int height)
		{
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new DynamicBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
			}
		}

		private GetTilemapOptions GetOptions(byte[]? prevVram = null)
		{
			return new GetTilemapOptions() {
				Layer = (byte)SelectedTab.Layer,
				CompareVram = prevVram,
				HighlightTileChanges = Config.HighlightTileChanges,
				HighlightAttributeChanges = Config.HighlightAttributeChanges,
				DisplayMode = Config.DisplayMode
			};
		}

		private void RefreshData<T>() where T : struct, BaseState
		{
			T ppuState = DebugApi.GetPpuState<T>(CpuType);
			_ppuState = ppuState;
			_prevVram = _vram;
			_vram = DebugApi.GetMemoryState(CpuType.GetVramMemoryType());
			_palette = PaletteHelper.GetConvertedPalette(CpuType, ConsoleType);

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
				
				GetTilemapOptions options = GetOptions(prevVram);

				FrameInfo size = DebugApi.GetTilemapSize(CpuType, options, ppuState);
				InitBitmap((int)size.Width, (int)size.Height);

				DebugTilemapInfo tilemapInfo;
				using(var framebuffer = ViewerBitmap.Lock()) {
					tilemapInfo = DebugApi.GetTilemap(CpuType, options, ppuState, vram, palette, framebuffer.FrameBuffer.Address);
				}

				UpdatePreviewPanel();

				if(Config.ShowScrollOverlay) {
					ScrollOverlayRect = new Rect(
						tilemapInfo.ScrollX % size.Width,
						tilemapInfo.ScrollY % size.Height,
						tilemapInfo.ScrollWidth,
						tilemapInfo.ScrollHeight
					);
				} else {
					ScrollOverlayRect = Rect.Empty;
				}
			});
		}

		public void RefreshData()
		{
			switch(CpuType) {
				case CpuType.Cpu: RefreshData<PpuState>(); break;
				case CpuType.Nes: RefreshData<NesPpuState>(); break;
				case CpuType.Gameboy: RefreshData<GbPpuState>(); break;
			}
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p, DynamicTooltip? tooltipToUpdate)
		{
			if(_ppuState == null || _prevVram == null) {
				return null;
			}

			DebugTilemapTileInfo? result = DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(), _prevVram, _ppuState);
			if(result == null) {
				return null;
			}

			DebugTilemapTileInfo tileInfo = result.Value;

			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			PixelRect cropRect = new PixelRect(p.X / tileInfo.Width * tileInfo.Width, p.Y / tileInfo.Height * tileInfo.Height, tileInfo.Width, tileInfo.Height);
			entries.AddPicture("Tile", ViewerBitmap, 6, cropRect);
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
			if(tileInfo.HorizontalMirroring != NullableBoolean.Undefined) {
				entries.AddEntry("Horizontal mirror", tileInfo.HorizontalMirroring == NullableBoolean.True);
			}
			if(tileInfo.VerticalMirroring != NullableBoolean.Undefined) {
				entries.AddEntry("Vertical mirror", tileInfo.VerticalMirroring == NullableBoolean.True);
			}
			if(tileInfo.HighPriority != NullableBoolean.Undefined) {
				entries.AddEntry("High priority", tileInfo.HighPriority == NullableBoolean.True);
			}

			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}
	}

	public class TilemapViewerTab
	{
		public string Title { get; set; } = "";
		public int Layer { get; set; }  = 0;
	}
}
