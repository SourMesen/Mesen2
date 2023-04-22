using Avalonia.Controls;
using Mesen.Config;
using Mesen.Localization;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.ViewModels
{
	public class SnesConfigViewModel : DisposableViewModel
	{
		[Reactive] public SnesConfig Config { get; set; }
		[Reactive] public SnesConfig OriginalConfig { get; set; }
		[Reactive] public SnesConfigTab SelectedTab { get; set; } = 0;

		public SnesInputConfigViewModel Input { get; private set; }

		[Reactive] public bool IsDefaultSpcClockSpeed { get; set; } = true;
		[Reactive] public string SpcEffectiveClockSpeed { get; set; } = "";

		public Enum[] AvailableRegions => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.Pal
		};

		public SnesConfigViewModel()
		{
			Config = ConfigManager.Config.Snes;
			OriginalConfig = Config.Clone();
			Input = new SnesInputConfigViewModel(Config);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(Input);
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));

			AddDisposable(this.WhenAnyValue(x => x.Config.SpcClockSpeedAdjustment).Subscribe(x => {
				SpcEffectiveClockSpeed = ResourceHelper.GetMessage("SpcClockSpeedMsg", ((32000 + x) * 32).ToString());
				IsDefaultSpcClockSpeed = x == 40;
			}));
		}
   }

	public enum SnesConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Overclocking,
		Video,
		Bsx
	}
}
