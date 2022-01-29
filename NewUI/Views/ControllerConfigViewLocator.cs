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
		public IControl Build(object data)
		{
			KeyMappingViewModel mappings = (KeyMappingViewModel)data;

			if(mappings != null) {
				return mappings.Type switch {
					ControllerType.SnesController or ControllerType.Multitap => new SnesControllerView(),
					ControllerType.NesController => new NesControllerView(),
					_ => new DefaultControllerView()
				};
			}

			return new TextBlock { Text = "No matching view found for controller type" };
		}

		public bool Match(object data)
		{
			return data is KeyMappingViewModel;
		}
	}
}
