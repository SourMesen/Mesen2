using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Mesen.Config;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Views
{
	public class ControllerConfigViewLocator : IDataTemplate
	{
		public Control Build(object? data)
		{
			KeyMappingViewModel? mappings = data as KeyMappingViewModel;

			if(mappings != null) {
				return mappings.Type switch {
					ControllerType.SnesController => new SnesControllerView(),
					ControllerType.NesController => new NesControllerView(),
					ControllerType.FamicomController => new NesControllerView(),
					ControllerType.FamicomControllerP2 => new NesControllerView(true),
					ControllerType.BandaiHyperShot => new BandaiHypershotControllerView(),
					ControllerType.HoriTrack => new NesControllerView(),
					ControllerType.GameboyController => new NesControllerView(),
					ControllerType.GbaController => new GbaControllerView(),
					ControllerType.PceController => new PceControllerView(),
					ControllerType.PceAvenuePad6 => new PceAvenuePad6View(),
					ControllerType.SmsController => new SmsControllerView(),
					ControllerType.WsController => new WsControllerView(),
					ControllerType.WsControllerVertical => new WsControllerVerticalView(),
					_ => new DefaultControllerView()
				};
			}

			return new TextBlock { Text = "No matching view found for controller type" };
		}

		public bool Match(object? data)
		{
			return data is KeyMappingViewModel;
		}
	}
}
