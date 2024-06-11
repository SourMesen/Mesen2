using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class RegisterViewerConfig : BaseWindowConfig<RegisterViewerConfig>
	{
		[Reactive] public RefreshTimingConfig RefreshTiming { get; set; } = new();
		[Reactive] public List<int> ColumnWidths { get; set; } = new();
	}
}
