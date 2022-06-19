using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class TileEditorConfig : BaseWindowConfig<TileEditorConfig>
	{
		[Reactive] public int ImageScale { get; set; } = 8;
		[Reactive] public TileBackground Background { get; set; } = TileBackground.Transparent;
	}
}
