using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class SpriteViewerConfig : BaseWindowConfig<SpriteViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;
		[Reactive] public int ImageScale { get; set; } = 2;
		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();
		[Reactive] public bool HideOffscreenSprites { get; set; } = false;

		public SpriteViewerConfig()
		{
		}
	}
}
