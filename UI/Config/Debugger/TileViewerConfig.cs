using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TileViewerConfig : BaseWindowConfig<TileViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public double ImageScale { get; set; } = 3;
		[Reactive] public bool ShowTileGrid { get; set; } = false;

		[Reactive] public string SelectedPreset { get; set; } = "PPU";
		
		[Reactive] public MemoryType Source { get; set; }
		[Reactive] public TileFormat Format { get; set; } = TileFormat.Bpp4;
		[Reactive] public TileLayout Layout { get; set; } = TileLayout.Normal;
		[Reactive] public TileFilter Filter { get; set; } = TileFilter.None;
		[Reactive] public TileBackground Background { get; set; } = TileBackground.Default;
		[Reactive] public int RowCount { get; set; } = 64;
		[Reactive] public int ColumnCount { get; set; } = 32;
		[Reactive] public int StartAddress { get; set; } = 0;
		[Reactive] public bool UseGrayscalePalette { get; set; } = false;

		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		public TileViewerConfig()
		{
		}
	}
}
