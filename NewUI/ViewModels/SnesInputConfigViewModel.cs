using Mesen.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class SnesInputConfigViewModel : DisposableViewModel
	{
		[Reactive] public SnesConfig Config { get; set; }
		
		[ObservableAsProperty] public bool HasMultitap1 { get; }
		[ObservableAsProperty] public bool HasMultitap2 { get; }

		public Enum[] AvailableControllerTypesP12 => new Enum[] {
			ControllerType.None,
			ControllerType.SnesController,
			ControllerType.SnesMouse,
			ControllerType.SuperScope,
			ControllerType.Multitap,
		};

		public Enum[] AvailableControllerTypesMultitap => new Enum[] {
			ControllerType.None,
			ControllerType.SnesController,
			ControllerType.SnesMouse,
			ControllerType.SuperScope,
		};

		[Obsolete("For designer only")]
		public SnesInputConfigViewModel() : this(new SnesConfig()) { }

		public SnesInputConfigViewModel(SnesConfig config)
		{
			Config = config;
		
			AddDisposable(this.WhenAnyValue(x => x.Config.Port1.Type)
				.Select(x => x == ControllerType.Multitap)
				.ToPropertyEx(this, x => x.HasMultitap1));

			AddDisposable(this.WhenAnyValue(x => x.Config.Port2.Type)
				.Select(x => x == ControllerType.Multitap)
				.ToPropertyEx(this, x => x.HasMultitap2));
		}
	}
}
