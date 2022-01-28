using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.ViewModels
{
	public class SnesConfigViewModel : DisposableViewModel
	{
		[Reactive] public SnesConfig Config { get; set; }

		public SnesInputConfigViewModel Input { get; private set; }

		public Enum[] AvailableRegions => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.Pal			
		};

		public SnesConfigViewModel()
		{
			Config = ConfigManager.Config.Snes.Clone();
			Input = new SnesInputConfigViewModel(Config);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
   }
}
