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
		public GameDipSwitches DipSwitches { get; }

		public GameConfigViewModel()
		{
			if(Design.IsDesignMode) {
				Config = new();
				DipSwitches = new();
				return;
			}

			Config = GameConfig.LoadGameConfig(MainWindowViewModel.Instance.RomInfo);
			DipSwitches = DipSwitchDatabase.GetGameDipswitches(MainWindowViewModel.Instance.RomInfo.DipSwitches);

			uint value = Config.DipSwitches;
			foreach(DipSwitchDefinition def in DipSwitches.DipSwitches) {
				def.SelectedOption = (int)(value & (def.Options.Count - 1));

				//Shift bits
				value /= (uint)def.Options.Count;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => Config.ApplyConfig()));
		}

		public void Save()
		{
			uint value = 0;
			for(int i = DipSwitches.DipSwitches.Count - 1; i >= 0; i--) {
				DipSwitchDefinition def = DipSwitches.DipSwitches[i];
				value *= (uint)def.Options.Count;

				value |= (uint)def.SelectedOption;
			}
			Config.DipSwitches = value;
			Config.Save();
		}
	}
}
