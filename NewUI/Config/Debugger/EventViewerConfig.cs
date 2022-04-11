using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class EventViewerConfig : BaseWindowConfig<EventViewerConfig>
	{
		[Reactive] public bool ShowSettingsPanel { get; set; } = true;

		[Reactive] public int ImageScale { get; set; } = 1;

		[Reactive] public bool RefreshOnBreakPause { get; set; } = true;
		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();

		[Reactive] public bool ShowToolbar { get; set; } = true;

		public SnesEventViewerConfig SnesConfig { get; set; } = new SnesEventViewerConfig();
		public NesEventViewerConfig NesConfig { get; set; } = new NesEventViewerConfig();
		public GbEventViewerConfig GbConfig { get; set; } = new GbEventViewerConfig();
		public PceEventViewerConfig PceConfig { get; set; } = new PceEventViewerConfig();
	}
}
