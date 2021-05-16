using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Dock.Model;
using Dock.Model.Core;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger
{
	class DebuggerViewLocator : IDataTemplate
	{
		public IControl Build(object data)
		{
			string? name = data.GetType().FullName?.Replace("ViewModel", "View");
			if(name == null) {
				return new TextBlock { Text = "Not Found: " + name };
			}

			Type? type = Type.GetType(name);
			if(type != null) {
				object? obj = Activator.CreateInstance(type);
				if(obj is Control) {
					return (Control)obj;
				} else {
					return new TextBlock();
				}
			} else {
				return new TextBlock { Text = "Not Found: " + name };
			}
		}

		public bool Match(object data)
		{
			return data is ViewModelBase || data is IDockable;
		}
	}
}
