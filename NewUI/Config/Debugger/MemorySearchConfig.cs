using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Config
{
	public class MemorySearchConfig : BaseWindowConfig<MemorySearchConfig>
	{
		[Reactive] public List<int> ColumnWidths { get; set; } = new();
	}
}
