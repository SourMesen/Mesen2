using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainWindowViewModel : ViewModelBase
	{
		[Reactive] public MainMenuViewModel? MainMenu { get; set; }
		[Reactive] public RomInfo RomInfo { get; set; }
		[Reactive] public AudioPlayerViewModel? AudioPlayer { get; private set; }
		[Reactive] public RecentGamesViewModel RecentGames { get; private set; }

		public MainWindowViewModel()
		{
			RomInfo = new RomInfo();
			RecentGames = new RecentGamesViewModel();
		}

		public void Init()
		{
			MainMenu = new MainMenuViewModel(this);
			RecentGames.Init(GameScreenMode.RecentGames);

			this.WhenAnyValue(x => x.RomInfo).Subscribe(x => {
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
