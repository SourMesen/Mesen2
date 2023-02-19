using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;
using System.Collections.Generic;
using System;

namespace Mesen.ViewModels
{
	public class AudioConfigViewModel : DisposableViewModel
	{
		[Reactive] public AudioConfig Config { get; set; }
		[Reactive] public AudioConfig OriginalConfig { get; set; }
		[Reactive] public List<string> AudioDevices { get; set; } = new();
		[Reactive] public bool ShowLatencyWarning { get; set; } = false;

		public AudioConfigViewModel()
		{
			Config = ConfigManager.Config.Audio;
			OriginalConfig = Config.Clone();

			if(Design.IsDesignMode) {
				return;
			}

			AudioDevices = ConfigApi.GetAudioDevices();
			if(AudioDevices.Count > 0 && !AudioDevices.Contains(Config.AudioDevice)) {
				Config.AudioDevice = AudioDevices[0];
			}

			AddDisposable(this.WhenAnyValue(x => x.Config.AudioLatency).Subscribe(x => {
				ShowLatencyWarning = Config.AudioLatency <= 55;
			}));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
	}
}
