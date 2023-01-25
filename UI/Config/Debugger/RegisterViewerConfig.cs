using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class RegisterViewerConfig : BaseWindowConfig<RegisterViewerConfig>
	{
		public RefreshTimingConfig RefreshTiming = new RefreshTimingConfig();
		[Reactive] public List<int> ColumnWidths { get; set; } = new();
	}
}
