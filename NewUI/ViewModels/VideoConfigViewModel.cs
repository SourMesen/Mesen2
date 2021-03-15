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
	public class VideoConfigViewModel : ViewModelBase
	{
		[Reactive] public VideoAspectRatio AspectRatio { get; set; }
		[ObservableAsProperty] public bool ShowCustomRatio { get; set; }

		[Reactive] public AudioConfig Config { get; set; }
		
		public VideoConfigViewModel()
		{
			Config = ConfigManager.Config.Audio;

			this.WhenAnyValue(_ => _.AspectRatio).Select(_ => _ == VideoAspectRatio.Custom).ToPropertyEx(this, _ => _.ShowCustomRatio);
		}
   }
}
