using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.ViewModels
{
	public class AudioConfigViewModel : DisposableViewModel
	{
		[Reactive] public AudioConfig Config { get; set; }
		[Reactive] public List<string> AudioDevices { get; set; } = new();
		
		public AudioConfigViewModel()
		{
			Config = ConfigManager.Config.Audio.Clone();

			if(Design.IsDesignMode) {
				return;
			}

			AudioDevices = ConfigApi.GetAudioDevices();
			if(AudioDevices.Count > 0 && !AudioDevices.Contains(Config.AudioDevice)) {
				Config.AudioDevice = AudioDevices[0];
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
	}
}
