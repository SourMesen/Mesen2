using Mesen.Config;
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
		[ObservableAsProperty] public bool ShowCustomRatio { get; }

		[Reactive] public VideoConfig Config { get; set; }
		
		public VideoConfigViewModel()
		{
			Config = ConfigManager.Config.Video.Clone();
			this.WhenAnyValue(_ => _.Config.AspectRatio).Select(_ => _ == VideoAspectRatio.Custom).ToPropertyEx(this, _ => _.ShowCustomRatio);
		}
   }
}
