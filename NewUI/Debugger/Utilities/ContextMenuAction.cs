using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Config;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.Debugger.Utilities
{
	public class ContextMenuAction : ViewModelBase
	{
		public ActionType ActionType;

		public string Name => ResourceHelper.GetEnumText(ActionType);
		public Image? Icon
		{
			get
			{
				IconFileAttribute? attr = ActionType.GetAttribute<IconFileAttribute>();
				if(!string.IsNullOrEmpty(attr?.Icon)) {
					return ImageUtilities.FromAsset(attr.Icon);
				}
				return null;
			}
		}

		public Func<bool>? IsEnabled { get; set; }

		public Func<DbgShortKeys>? Shortcut { get; set; }
		public string ShortcutText => Shortcut?.Invoke().ToString() ?? "";


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
				_onClick = () => {
					if(IsEnabled == null || IsEnabled()) {
						value();
					}
				};
				_clickCommand = ReactiveCommand.Create(_onClick, this.WhenAnyValue(x => x.Enabled));
			}
		}
	}

	public enum ActionType
	{
		[IconFile("Copy")]
		Copy,

		[IconFile("Paste")]
		Paste,

		[IconFile("SelectAll")]
		SelectAll
	}
}
