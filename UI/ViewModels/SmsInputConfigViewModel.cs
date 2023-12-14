using Avalonia.Threading;
using Mesen.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class SmsInputConfigViewModel : DisposableViewModel
	{
		[Reactive] public SmsConfig Config { get; set; }

		public Enum[] AvailableControllerTypesP12 => new Enum[] {
			ControllerType.None,
			ControllerType.SmsController,
			ControllerType.SmsLightPhaser,
		};

		[Obsolete("For designer only")]
		public SmsInputConfigViewModel() : this(new SmsConfig()) { }

		public SmsInputConfigViewModel(SmsConfig config)
		{
			Config = config;
		}
	}
}
