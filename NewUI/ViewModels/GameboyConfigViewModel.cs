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
	public class GameboyConfigViewModel : ViewModelBase
	{
		[Reactive] public GameboyConfig Config { get; set; }

		public GameboyConfigViewModel()
		{
			Config = ConfigManager.Config.Gameboy.Clone();
		}
   }
}
