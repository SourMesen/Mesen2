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
	public class PreferencesConfigViewModel : ViewModelBase
	{
		[Reactive] public PreferencesConfig Config { get; set; }
		
		public PreferencesConfigViewModel()
		{
			Config = new PreferencesConfig();
		}
   }
}
