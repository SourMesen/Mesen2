using Mesen.GUI.Config;
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
	public class SnesConfigViewModel : ViewModelBase
	{
		[Reactive] public SnesConfig Config { get; set; }

		public SnesInputConfigViewModel Input { get; private set; }

		public Enum[] AvailableRegions => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.Pal			
		};

		public SnesConfigViewModel()
		{
			Config = ConfigManager.Config.Snes.Clone();
			Input = new SnesInputConfigViewModel(Config);
		}
   }
}
