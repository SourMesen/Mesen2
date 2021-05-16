using Mesen.GUI.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class ConfigViewModel : ViewModelBase
	{
		[Reactive] public AudioConfigViewModel? Audio { get; set; }
		[Reactive] public VideoConfigViewModel? Video { get; set; }
		[Reactive] public PreferencesConfigViewModel? Preferences { get; set; }
		[Reactive] public EmulationConfigViewModel? Emulation { get; set; }
		
		[Reactive] public SnesConfigViewModel? Snes { get; set; }
		[Reactive] public NesConfigViewModel? Nes { get; set; }
		[Reactive] public GameboyConfigViewModel? Gameboy { get; set; }

		[Reactive] public ConfigWindowTab SelectedIndex { get; set; }

		//For designer
		public ConfigViewModel() : this(ConfigWindowTab.Audio) { }

		public ConfigViewModel(ConfigWindowTab selectedTab)
		{
			this.SelectedIndex = selectedTab;

			this.WhenAnyValue(x => x.SelectedIndex).Subscribe((tab) => {
				this.SelectTab(tab);
			});
		}

		public void SelectTab(ConfigWindowTab tab)
		{
			//Create each view model when the corresponding tab is clicked, for performance
			switch(tab) {
				case ConfigWindowTab.Audio: Audio ??= new AudioConfigViewModel(); break;
				case ConfigWindowTab.Emulation: Emulation ??= new EmulationConfigViewModel(); break;
				case ConfigWindowTab.Video: Video ??= new VideoConfigViewModel(); break;

				case ConfigWindowTab.Nes:
					Preferences ??= new PreferencesConfigViewModel();
					Nes ??= new NesConfigViewModel(Preferences.Config);
					break;

				case ConfigWindowTab.Snes: Snes ??= new SnesConfigViewModel(); break;
				case ConfigWindowTab.Gameboy: Gameboy ??= new GameboyConfigViewModel(); break;

				case ConfigWindowTab.Preferences: Preferences ??= new PreferencesConfigViewModel(); break;
			}

			this.SelectedIndex = tab;
		}

		public void ApplyConfig()
		{
			Audio?.Config.ApplyConfig();
			Video?.Config.ApplyConfig();
			Preferences?.Config.ApplyConfig();
			Emulation?.Config.ApplyConfig();
			Nes?.Config.ApplyConfig();
			Snes?.Config.ApplyConfig();
			Gameboy?.Config.ApplyConfig();
		}

		public void SaveConfig()
		{
			if(Preferences != null && ConfigManager.Config.Preferences.Theme != Preferences.Config.Theme) {
				PreferencesConfig.ApplyTheme(Preferences.Config.Theme);
			}

			ConfigManager.Config.Audio = Audio?.Config.Clone() ?? ConfigManager.Config.Audio;
			ConfigManager.Config.Video = Video?.Config.Clone() ?? ConfigManager.Config.Video;
			ConfigManager.Config.Preferences = Preferences?.Config.Clone() ?? ConfigManager.Config.Preferences;
			ConfigManager.Config.Emulation = Emulation?.Config.Clone() ?? ConfigManager.Config.Emulation;
			ConfigManager.Config.Nes = Nes?.Config.Clone() ?? ConfigManager.Config.Nes;
			ConfigManager.Config.Snes = Snes?.Config.Clone() ?? ConfigManager.Config.Snes;
			ConfigManager.Config.Gameboy = Gameboy?.Config.Clone() ?? ConfigManager.Config.Gameboy;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.SaveConfig();

			ConfigManager.Config.Preferences.UpdateFileAssociations();
		}
   }

	public enum ConfigWindowTab
	{
		Audio = 0,
		Emulation = 1,
		Video = 2,

		Nes = 4,
		Snes = 5,
		Gameboy = 6,

		Preferences = 8
	}
}
