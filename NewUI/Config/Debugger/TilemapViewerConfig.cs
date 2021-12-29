using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TilemapViewerConfig : BaseWindowConfig<TilemapViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public int ImageScale { get; set; } = 1;

		[Reactive] public bool ShowGrid { get; set; }
		[Reactive] public bool ShowAltGrid { get; set; }
		[Reactive] public bool ShowScrollOverlay { get; set; }
		[Reactive] public bool HighlightTileChanges { get; set; }
		[Reactive] public bool HighlightAttributeChanges { get; set; }
		[Reactive] public TilemapDisplayMode DisplayMode { get; set; } = TilemapDisplayMode.Default;

		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		public TilemapViewerConfig()
		{
		}
	}
}
