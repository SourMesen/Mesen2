using Mesen.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class EmulationConfigViewModel : ViewModelBase
	{
		[Reactive] public EmulationConfig Config { get; set; }
		
		public EmulationConfigViewModel()
		{
			Config = ConfigManager.Config.Emulation.Clone();
		}
   }
}
