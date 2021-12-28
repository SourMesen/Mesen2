using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.ViewModels
{
	public class TilemapViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		public TilemapViewerConfig Config { get; }

		[Reactive] public Rect SelectionRect { get; set; }

		[Reactive] public WriteableBitmap ViewerBitmap { get; private set; }
		[Reactive] public BaseState? PpuState { get; set; }
		[Reactive] public byte[]? PrevVram { get; set; }

		[Reactive] public DynamicTooltip? PreviewPanel { get; private set; }

		[Reactive] public List<TilemapViewerTab> Tabs { get; private set; } = new List<TilemapViewerTab>();
		[Reactive] public bool ShowTabs { get; private set; }
		[Reactive] public TilemapViewerTab SelectedTab { get; set; }

		//For designer
		public TilemapViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public TilemapViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			Config = ConfigManager.Config.Debug.TilemapViewer;
			CpuType = cpuType;
			ConsoleType = consoleType;

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

			this.WhenAnyValue(x => x.Tabs).Subscribe(x => ShowTabs = x.Count > 1);

			this.WhenAnyValue(x => x.SelectionRect).Subscribe(x => {
				if(x.IsEmpty) {
					PreviewPanel = null;
				} else {
					PreviewPanel = GetPreviewPanel(PixelPoint.FromPoint(x.TopLeft, 1));
				}
			});
		}

		public void InitBitmap(int width, int height)
		{
			if(ViewerBitmap == null || ViewerBitmap.PixelSize.Width != width || ViewerBitmap.PixelSize.Height != height) {
				ViewerBitmap = new WriteableBitmap(new PixelSize(width, height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
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

		public void UpdateBitmap<T>(T ppuState, byte[] vram, byte[]? prevVram) where T : struct, BaseState
		{
			GetTilemapOptions options = GetOptions(prevVram);
			UInt32[] palette = PaletteHelper.GetConvertedPalette(CpuType, ConsoleType);

			FrameInfo size = DebugApi.GetTilemapSize(CpuType, options, ppuState);
			InitBitmap((int)size.Width, (int)size.Height);

			using(var framebuffer = ViewerBitmap.Lock()) {
				DebugApi.GetTilemap(CpuType, options, ppuState, vram, palette, framebuffer.Address);
			}
		}

		public DynamicTooltip? GetPreviewPanel(PixelPoint p)
		{
			if(PpuState == null || PrevVram == null) {
				return null;
			}

			DebugTilemapTileInfo? result = DebugApi.GetTilemapTileInfo((uint)p.X, (uint)p.Y, CpuType, GetOptions(), PrevVram, PpuState);
			if(result == null) {
				return null;
			}

			DebugTilemapTileInfo tileInfo = result.Value;
			CroppedBitmap preview = new CroppedBitmap(ViewerBitmap, new PixelRect(p.X / tileInfo.Width * tileInfo.Width, p.Y / tileInfo.Height * tileInfo.Height, tileInfo.Width, tileInfo.Height));

			List<TooltipEntry> entries = new();
			entries.Add(new("Tile", new Border() { 
				BorderBrush = Brushes.Gray, 
				BorderThickness = new Thickness(1), 
				Child = new PictureViewer() { 
					Source = preview,
					Width = tileInfo.Width * 6,
					Height = tileInfo.Height * 6,
					Zoom = 6
				}
			}));
			entries.Add(new("Column, Row", tileInfo.Column + ", " + tileInfo.Row));
			entries.Add(new("Size", tileInfo.Width + "x" + tileInfo.Height));

			if(tileInfo.TileMapAddress >= 0) {
				entries.Add(new("Tilemap address", "$" + tileInfo.TileMapAddress.ToString("X4")));
			}
			if(tileInfo.TileIndex >= 0) {
				entries.Add(new("Tile index", "$" + tileInfo.TileIndex.ToString("X2")));
			}
			if(tileInfo.TileAddress >= 0) {
				entries.Add(new("Tile address", "$" + tileInfo.TileAddress.ToString("X4")));
			}
			if(tileInfo.PaletteIndex >= 0) {
				entries.Add(new("Palette index", tileInfo.PaletteIndex.ToString()));
			}
			if(tileInfo.PaletteAddress >= 0) {
				entries.Add(new("Palette address", "$" + tileInfo.PaletteAddress.ToString("X2")));
			}
			if(tileInfo.HorizontalMirroring != NullableBoolean.Undefined) {
				entries.Add(new("Horizontal mirror", tileInfo.HorizontalMirroring == NullableBoolean.True));
			}
			if(tileInfo.VerticalMirroring != NullableBoolean.Undefined) {
				entries.Add(new("Vertical mirror", tileInfo.VerticalMirroring == NullableBoolean.True));
			}
			if(tileInfo.HighPriority != NullableBoolean.Undefined) {
				entries.Add(new("High priority", tileInfo.HighPriority == NullableBoolean.True));
			}

			return new DynamicTooltip() { Items = entries };
		}
	}

	public class TilemapViewerTab
	{
		public string Title { get; set; } = "";
		public int Layer { get; set; }  = 0;
	}
}
