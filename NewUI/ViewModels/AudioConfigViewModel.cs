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
	public class AudioConfigViewModel : ViewModelBase
	{
		[Reactive] public AudioConfig Config { get; set; }
		
		public AudioConfigViewModel()
		{
			Config = ConfigManager.Config.Audio;
		}
   }
}
