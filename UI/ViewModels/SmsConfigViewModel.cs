using Avalonia.Controls;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class SmsConfigViewModel : DisposableViewModel
	{
		[Reactive] public SmsConfig Config { get; set; }
		[Reactive] public SmsConfig OriginalConfig { get; set; }
		[Reactive] public SmsConfigTab SelectedTab { get; set; } = 0;

		public SmsInputConfigViewModel Input { get; private set; }
		
		public Enum[] AvailableRegionsSms => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.Pal
		};

		public Enum[] AvailableRegionsGg => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.NtscJapan,
			ConsoleRegion.Pal
		};

		public SmsConfigViewModel()
		{
			Config = ConfigManager.Config.Sms;
			OriginalConfig = Config.Clone();
			Input = new SmsInputConfigViewModel(Config);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(Input);
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
	}

	public enum SmsConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Video
	}
}
