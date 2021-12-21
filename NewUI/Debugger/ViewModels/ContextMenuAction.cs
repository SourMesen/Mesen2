using Avalonia.Controls;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.Debugger.Controls
{
	public class ContextMenuAction : ViewModelBase
	{
		public string? Name { get; set; }
		public Func<bool>? IsEnabled { get; set; }

		public string? IconFile { get; set; }
		public Image? Icon => IconFile != null ? ImageUtilities.FromAsset(IconFile) : null;

		[Reactive] public bool Enabled { get; set; }

		private ReactiveCommand<Unit, Unit>? _clickCommand;
		public ReactiveCommand<Unit, Unit>? ClickCommand 
		{
			get
			{
				Enabled = IsEnabled?.Invoke() ?? true;
				return _clickCommand;
			}
		}

		private Action _onClick = () => { };
		public Action OnClick
		{
			get => _onClick;
			set {
				_onClick = value;
				_clickCommand = ReactiveCommand.Create(_onClick, this.WhenAnyValue(x => x.Enabled));
			}
		}
	}
}
