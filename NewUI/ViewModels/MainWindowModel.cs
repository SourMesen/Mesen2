using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainWindowViewModel : ViewModelBase
	{
		[Reactive] public RomInfo RomInfo { get; set; }
		
		[Reactive] public bool IsGameRunning { get; private set; }
		[Reactive] public bool IsFdsGame { get; private set; }
		[Reactive] public bool IsNesGame { get; private set; }
		[Reactive] public AudioPlayerViewModel? AudioPlayer { get; private set; }

		public MainWindowViewModel()
		{
			this.RomInfo = new RomInfo();

			this.WhenAnyValue(x => x.RomInfo).Subscribe(x => {
				IsGameRunning = x.Format != RomFormat.Unknown;

				IsFdsGame = x.Format == RomFormat.Fds;
				IsNesGame = x.Format == RomFormat.iNes || x.Format == RomFormat.Fds || x.Format == RomFormat.Unif;
				
				bool showAudioPlayer = x.Format == RomFormat.Nsf || x.Format == RomFormat.Spc || x.Format == RomFormat.Gbs;
				if(AudioPlayer == null && showAudioPlayer) {
					AudioPlayer = new AudioPlayerViewModel();
				} else if(!showAudioPlayer) {
					AudioPlayer = null;
				}
			});
		}
	}
}
