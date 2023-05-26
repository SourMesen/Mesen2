using Dock.Model.Mvvm.Controls;
using Mesen.Debugger.Utilities;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.ViewModels.DebuggerDock
{
	public class BaseToolContainerViewModel : Tool
	{
		public virtual object? HelpContent { get; } = null;

		public event EventHandler? Selected;

		public override void OnSelected()
		{
			Selected?.Invoke(this, EventArgs.Empty);
		}
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
			CanFloat = false;
		}
	}
}
