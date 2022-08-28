using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using Microsoft.VisualBasic;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.ViewModels
{
	public class GameConfigViewModel : DisposableViewModel
	{
		[Reactive] public GameConfig Config { get; set; }

		public GameConfigViewModel()
		{
			if(Design.IsDesignMode) {
				Config = new();
				return;
			}

			Config = GameConfig.LoadGameConfig(MainWindowViewModel.Instance.RomInfo.GetRomName());
			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => Config.ApplyConfig()));
		}

		public void Save()
		{
			Config.Save();
		}
	}
}
