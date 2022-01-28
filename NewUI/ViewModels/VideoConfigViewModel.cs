using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class VideoConfigViewModel : DisposableViewModel
	{
		[ObservableAsProperty] public bool ShowCustomRatio { get; }

		[Reactive] public VideoConfig Config { get; set; }
		
		public VideoConfigViewModel()
		{
			Config = ConfigManager.Config.Video.Clone();
			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
			AddDisposable(this.WhenAnyValue(_ => _.Config.AspectRatio).Select(_ => _ == VideoAspectRatio.Custom).ToPropertyEx(this, _ => _.ShowCustomRatio));
		}
   }
}
