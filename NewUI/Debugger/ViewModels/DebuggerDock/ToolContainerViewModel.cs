using Dock.Model.ReactiveUI.Controls;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class ToolContainerViewModel<T> : Tool
	{
		[Reactive] public T? Model { get; set; }

		public ToolContainerViewModel(string name)
		{
			Id = name;
			Title = name;
			CanPin = false;
		}
	}
}
