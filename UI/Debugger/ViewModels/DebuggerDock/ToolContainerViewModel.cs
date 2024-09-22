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
		private T? _model;

		public T? Model 
		{ 
			get => _model;
			set
			{
				_model = value;
				OnPropertyChanged(nameof(Model));
			}
		}

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
