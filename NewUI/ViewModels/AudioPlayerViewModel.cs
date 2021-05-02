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
	public class AudioPlayerViewModel : ViewModelBase
	{
		[Reactive] public AudioPlayerConfig Config { get; set; }
		
		public AudioPlayerViewModel()
		{
			Config = ConfigManager.Config.AudioPlayer;

			this.WhenAnyValue(x => x.Config.Volume).Subscribe((vol) => {
				Config.ApplyConfig();
			});
		}
   }
}
