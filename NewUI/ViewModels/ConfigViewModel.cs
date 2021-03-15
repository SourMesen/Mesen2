using Mesen.GUI.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class ConfigViewModel : ViewModelBase
	{
		[Reactive] public AudioConfigViewModel Audio { get; set; }
		[Reactive] public VideoConfigViewModel Video { get; set; }
		
		public ConfigViewModel()
		{
			this.Audio = new AudioConfigViewModel();
			this.Video = new VideoConfigViewModel();
		}
   }
}
