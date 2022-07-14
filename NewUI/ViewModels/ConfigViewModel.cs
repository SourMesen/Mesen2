using Mesen.Config;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class ConfigViewModel : DisposableViewModel
	{
		[Reactive] public AudioConfigViewModel? Audio { get; set; }
		[Reactive] public InputConfigViewModel? Input { get; set; }
		[Reactive] public VideoConfigViewModel? Video { get; set; }
		[Reactive] public PreferencesConfigViewModel? Preferences { get; set; }
		[Reactive] public EmulationConfigViewModel? Emulation { get; set; }
		
		[Reactive] public SnesConfigViewModel? Snes { get; set; }
		[Reactive] public NesConfigViewModel? Nes { get; set; }
		[Reactive] public GameboyConfigViewModel? Gameboy { get; set; }
		[Reactive] public PceConfigViewModel? PcEngine { get; set; }

		[Reactive] public ConfigWindowTab SelectedIndex { get; set; }
		public bool AlwaysOnTop { get; }

		[Obsolete("For designer only")]
		public ConfigViewModel() : this(ConfigWindowTab.Audio) { }

		public ConfigViewModel(ConfigWindowTab selectedTab)
		{
			AlwaysOnTop = ConfigManager.Config.Preferences.AlwaysOnTop;
			SelectedIndex = selectedTab;

			AddDisposable(this.WhenAnyValue(x => x.SelectedIndex).Subscribe((tab) => {
				this.SelectTab(tab);
			}));
		}

		public void SelectTab(ConfigWindowTab tab)
		{
			//Create each view model when the corresponding tab is clicked, for performance
			switch(tab) {
				case ConfigWindowTab.Audio: Audio ??= AddDisposable(new AudioConfigViewModel()); break;
				case ConfigWindowTab.Emulation: Emulation ??= AddDisposable(new EmulationConfigViewModel()); break;
				case ConfigWindowTab.Input: Input ??= AddDisposable(new InputConfigViewModel()); break;
				case ConfigWindowTab.Video: Video ??= AddDisposable(new VideoConfigViewModel()); break;

				case ConfigWindowTab.Nes:
					//TODO fix this patch
					Preferences ??= AddDisposable(new PreferencesConfigViewModel());
					Nes ??= AddDisposable(new NesConfigViewModel(Preferences.Config));
					break;

				case ConfigWindowTab.Snes: Snes ??= AddDisposable(new SnesConfigViewModel()); break;
				case ConfigWindowTab.Gameboy: Gameboy ??= AddDisposable(new GameboyConfigViewModel()); break;
				case ConfigWindowTab.PcEngine: PcEngine ??= AddDisposable(new PceConfigViewModel()); break;

				case ConfigWindowTab.Preferences: Preferences ??= AddDisposable(new PreferencesConfigViewModel()); break;
			}

			SelectedIndex = tab;
		}

		public void SaveConfig()
		{
			ConfigManager.Config.Audio = Audio?.Config.Clone() ?? ConfigManager.Config.Audio;
			ConfigManager.Config.Input = Input?.Config.Clone() ?? ConfigManager.Config.Input;
			ConfigManager.Config.Video = Video?.Config.Clone() ?? ConfigManager.Config.Video;
			ConfigManager.Config.Preferences = Preferences?.Config.Clone() ?? ConfigManager.Config.Preferences;
			ConfigManager.Config.Emulation = Emulation?.Config.Clone() ?? ConfigManager.Config.Emulation;
			ConfigManager.Config.Nes = Nes?.Config.Clone() ?? ConfigManager.Config.Nes;
			ConfigManager.Config.Snes = Snes?.Config.Clone() ?? ConfigManager.Config.Snes;
			ConfigManager.Config.Gameboy = Gameboy?.Config.Clone() ?? ConfigManager.Config.Gameboy;
			ConfigManager.Config.PcEngine = PcEngine?.Config.Clone() ?? ConfigManager.Config.PcEngine;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.Config.Save();

			ConfigManager.Config.Preferences.UpdateFileAssociations();
		}
   }

	public enum ConfigWindowTab
	{
		Audio = 0,
		Emulation = 1,
		Input = 2,
		Video = 3,
		//separator
		Nes = 5,
		Snes = 6,
		Gameboy = 7,
		PcEngine = 8,
		//separator
		Preferences = 10
	}
}
