using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class PaletteViewerConfig : BaseWindowConfig<PaletteViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;
		[Reactive] public bool ShowPaletteIndexes { get; set; } = false;
		[Reactive] public int Zoom { get; set; } = 3;

		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		public PaletteViewerConfig()
		{
		}
	}
}
