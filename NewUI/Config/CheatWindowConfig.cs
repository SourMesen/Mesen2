using Avalonia;
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
	}
}
