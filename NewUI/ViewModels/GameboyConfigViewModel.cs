using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;

namespace Mesen.ViewModels
{
	public class GameboyConfigViewModel : DisposableViewModel
	{
		[Reactive] public GameboyConfig Config { get; set; }

		public GameboyConfigViewModel()
		{
			Config = ConfigManager.Config.Gameboy.Clone();
			
			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
   }
}
