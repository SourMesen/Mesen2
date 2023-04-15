using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TileEditorConfig : BaseWindowConfig<TileEditorConfig>
	{
		[Reactive] public double ImageScale { get; set; } = 8;
		[Reactive] public bool ShowGrid { get; set; } = false;
		[Reactive] public TileBackground Background { get; set; } = TileBackground.Transparent;
	}
}
