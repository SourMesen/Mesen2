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
	public class InputConfigViewModel : ViewModelBase
	{
		[Reactive] public InputConfig Config { get; set; }
		
		public InputConfigViewModel()
		{
			Config = ConfigManager.Config.Input.Clone();
		}
   }
}
