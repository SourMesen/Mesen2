using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Config
{
	public class HistoryViewerConfig : BaseWindowConfig<HistoryViewerConfig>
	{
		[Reactive] public int Scale { get; set; } = 2;
	}
}
