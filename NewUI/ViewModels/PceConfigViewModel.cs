using Avalonia.Controls;
using Mesen.Config;
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
	public class PceConfigViewModel : DisposableViewModel
	{
		[Reactive] public PcEngineConfig Config { get; set; }
		[Reactive] public PceConfigTab SelectedTab { get; set; } = 0;

		public PceInputConfigViewModel Input { get; private set; }

		public PceConfigViewModel()
		{
			Config = ConfigManager.Config.PcEngine.Clone();
			Input = new PceInputConfigViewModel(Config);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(Input);
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
	}

	public enum PceConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Video
	}
}
