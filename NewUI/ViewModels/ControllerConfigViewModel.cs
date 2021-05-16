using Mesen.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class ControllerConfigViewModel : ViewModelBase
	{
		[Reactive] public ControllerConfig Config { get; set; }

		[Reactive] public KeyMappingViewModel KeyMapping1 { get; set; }
		[Reactive] public KeyMappingViewModel KeyMapping2 { get; set; }
		[Reactive] public KeyMappingViewModel KeyMapping3 { get; set; }
		[Reactive] public KeyMappingViewModel KeyMapping4 { get; set; }

		//For designer
		public ControllerConfigViewModel() : this(new ControllerConfig()) { }

		public ControllerConfigViewModel(ControllerConfig config)
		{
			Config = config;

			KeyMapping1 = new KeyMappingViewModel(config.Type, config.Mapping1);
			KeyMapping2 = new KeyMappingViewModel(config.Type, config.Mapping2);
			KeyMapping3 = new KeyMappingViewModel(config.Type, config.Mapping3);
			KeyMapping4 = new KeyMappingViewModel(config.Type, config.Mapping4);
		}
	}

	public class KeyMappingViewModel : ViewModelBase
	{
		[Reactive] public ControllerType Type { get; set; }
		[Reactive] public KeyMapping Mapping { get; set; }

		public KeyMappingViewModel(ControllerType type, KeyMapping mapping)
		{
			Type = type;
			Mapping = mapping;
		}
	}
}
