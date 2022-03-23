using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Utilities;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class BaseToolContainerViewModel : Tool
	{
		public virtual object? HelpContent { get; } = null;
	}

	public class ToolContainerViewModel<T> : BaseToolContainerViewModel
	{
		[Reactive] public T? Model { get; set; }

		public override object? HelpContent
		{
			get
			{
				if(Model is IToolHelpTooltip help) {
					return help.HelpTooltip;
				}
				return null;
			}
		}

		public ToolContainerViewModel(string name)
		{
			Id = name;
			Title = name;
			CanPin = false;
		}
	}
}
