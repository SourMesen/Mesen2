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
		[Reactive] public AudioConfigViewModel Audio { get; set; }
		[Reactive] public VideoConfigViewModel Video { get; set; }
		[Reactive] public PreferencesConfigViewModel Preferences { get; set; }
		[Reactive] public EmulationConfigViewModel Emulation { get; set; }
		[Reactive] public InputConfigViewModel Input { get; set; }

		public ConfigViewModel()
		{
			this.Audio = new AudioConfigViewModel();
			this.Video = new VideoConfigViewModel();
			this.Preferences = new PreferencesConfigViewModel();
			this.Emulation = new EmulationConfigViewModel();
			this.Input = new InputConfigViewModel();
		}

		public void ApplyConfig()
		{
			this.Audio.Config.ApplyConfig();
			this.Video.Config.ApplyConfig();
			this.Preferences.Config.ApplyConfig();
			this.Emulation.Config.ApplyConfig();
			this.Input.Config.ApplyConfig();
		}

		public void SaveConfig()
		{
			ConfigManager.Config.Audio = this.Audio.Config;
			ConfigManager.Config.Video = this.Video.Config;
			ConfigManager.Config.Preferences = this.Preferences.Config;
			ConfigManager.Config.Emulation = this.Emulation.Config;
			ConfigManager.Config.Input = this.Input.Config;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.SaveConfig();
		}
   }
}
