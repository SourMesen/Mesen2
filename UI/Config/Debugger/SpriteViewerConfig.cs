using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class SpriteViewerConfig : BaseWindowConfig<SpriteViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public bool ShowOutline { get; set; } = false;
		[Reactive] public bool ShowOffscreenRegions { get; set; } = false;
		[Reactive] public SpriteBackground Background { get; set; } = SpriteBackground.Gray;

		[Reactive] public SpriteViewerSource Source { get; set; } = SpriteViewerSource.SpriteRam;
		[Reactive] public int SourceOffset { get; set; } = 0;

		[Reactive] public bool DimOffscreenSprites { get; set; } = true;
		[Reactive] public bool ShowListView { get; set; } = false;
		[Reactive] public double ListViewHeight { get; set; } = 100;
		[Reactive] public List<int> ColumnWidths { get; set; } = new();

		[Reactive] public double ImageScale { get; set; } = 2;
		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		public SpriteViewerConfig()
		{
		}
	}

	public enum SpriteViewerSource
	{
		SpriteRam,
		CpuMemory
	}
}
