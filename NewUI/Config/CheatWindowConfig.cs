using Avalonia;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class CheatWindowConfig : BaseWindowConfig<CheatWindowConfig>
	{
		public bool DisableAllCheats { get; set; } = false;
		public List<int> ColumnWidths { get; set; } = new();
	}
}
