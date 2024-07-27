using Avalonia.Threading;
using Mesen.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class CvInputConfigViewModel : DisposableViewModel
	{
		[Reactive] public CvConfig Config { get; set; }

		public Enum[] AvailableControllerTypesP12 => new Enum[] {
			ControllerType.None,
			ControllerType.ColecoVisionController,
		};

		[Obsolete("For designer only")]
		public CvInputConfigViewModel() : this(new CvConfig()) { }

		public CvInputConfigViewModel(CvConfig config)
		{
			Config = config;
		}
	}
}
