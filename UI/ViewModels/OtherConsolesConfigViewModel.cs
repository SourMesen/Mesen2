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
	public class OtherConsolesConfigViewModel : DisposableViewModel
	{
		[Reactive] public CvConfig CvConfig { get; set; }
		[Reactive] public CvConfig CvOriginalConfig { get; set; }
		[Reactive] public OtherConsolesConfigTab SelectedTab { get; set; } = 0;

		public CvInputConfigViewModel CvInput { get; private set; }
		
		public Enum[] AvailableRegionsCv => new Enum[] {
			ConsoleRegion.Auto,
			ConsoleRegion.Ntsc,
			ConsoleRegion.Pal
		};

		public OtherConsolesConfigViewModel()
		{
			CvConfig = ConfigManager.Config.Cv;
			CvOriginalConfig = CvConfig.Clone();
			CvInput = new CvInputConfigViewModel(CvConfig);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(CvInput);
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(CvConfig, (s, e) => { CvConfig.ApplyConfig(); }));
		}
	}

	public enum OtherConsolesConfigTab
	{
		ColecoVision
	}
}
