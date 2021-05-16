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
		[Reactive] public RomInfo RomInfo { get; set; }
		
		[Reactive] public bool IsGameRunning { get; private set; }
		[Reactive] public bool IsFdsGame { get; private set; }
		[Reactive] public bool IsNesGame { get; private set; }
		[Reactive] public AudioPlayerViewModel? AudioPlayer { get; private set; }

		[Reactive] public ObservableCollection<RecentItem> RecentItems { get; private set; }
		[Reactive] public bool HasRecentItems { get; private set; }
		public ReactiveCommand<RecentItem, Unit> OpenRecentCommand { get; }

		public MainWindowViewModel()
		{
			OpenRecentCommand = ReactiveCommand.Create<RecentItem>(OpenRecent);

			RomInfo = new RomInfo();
			RecentItems = ConfigManager.Config.RecentFiles.Items;

			this.WhenAnyValue(x => x.RecentItems.Count).Subscribe(count => {
				HasRecentItems = count > 0;
			});

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

		private void OpenRecent(RecentItem recent)
		{
			//Avalonia bug - Run in another thread to allow menu to close properly
			//See: https://github.com/AvaloniaUI/Avalonia/issues/5376
			Task.Run(() => {
				LoadRomHelper.LoadRom(recent.RomFile, recent.PatchFile);
			});
		}
	}
}
