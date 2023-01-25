using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Dock.Model.Core;
using Mesen.Debugger.Views.DebuggerDock;
using Mesen.ViewModels;
using System;

namespace Mesen.Debugger
{
	class DebuggerViewLocator : IDataTemplate
	{
		public IControl Build(object? data)
		{
			string? name = data?.GetType().FullName?.Replace("ViewModel", "View");
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
				if(data?.GetType().Name.StartsWith(nameof(ToolContainerView)) == true) {
					return new ToolContainerView();
				} else {
					return new TextBlock { Text = "Not Found: " + name };
				}
			}
		}

		public bool Match(object? data)
		{
			return data is ViewModelBase || data is IDockable;
		}
	}
}
