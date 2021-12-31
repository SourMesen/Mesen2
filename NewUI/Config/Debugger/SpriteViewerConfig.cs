using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class SpriteViewerConfig : BaseWindowConfig<SpriteViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public bool ShowCoordsInHex { get; set; } = false;
		[Reactive] public bool ShowOutline { get; set; } = false;
		[Reactive] public bool HideOffscreenSprites { get; set; } = false;

		[Reactive] public int ImageScale { get; set; } = 2;
		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		public SpriteViewerConfig()
		{
		}
	}
}
