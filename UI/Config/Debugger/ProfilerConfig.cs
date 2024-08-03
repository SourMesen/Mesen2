using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class ProfilerConfig : BaseWindowConfig<ProfilerConfig>
	{
		[Reactive] public List<int> ColumnWidths { get; set; } = new();
		[Reactive] public bool AutoRefresh { get; set; } = true;
		[Reactive] public bool RefreshOnBreakPause { get; set; } = true;
	}
}