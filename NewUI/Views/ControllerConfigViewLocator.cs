using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Mesen.GUI.Config;
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
				Control? ctrl = null;
				switch(mappings.Type) {
					case ControllerType.SnesController: ctrl = new SnesControllerView(); break;
					case ControllerType.NesController: ctrl = new NesControllerView(); break;
				}
				if(ctrl != null) {
					ctrl.DataContext = mappings.Mapping;
					return ctrl;
				}
			}

			return new TextBlock { Text = "No matching view found for controller type" };
		}

		public bool Match(object data)
		{
			return data is KeyMappingViewModel;
		}
	}
}
