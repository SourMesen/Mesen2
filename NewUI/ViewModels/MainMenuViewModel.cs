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
	public class MainMenuViewModel : ViewModelBase
	{
		public MainWindowViewModel MainWindow { get; set; }

		[Reactive] public bool IsGameRunning { get; private set; }
		[Reactive] public bool IsFdsGame { get; private set; }
		[Reactive] public bool IsVsSystemGame { get; private set; }
		[Reactive] public bool IsVsDualSystemGame { get; private set; }
		[Reactive] public bool IsNesGame { get; private set; }
		[Reactive] public bool IsGbGame { get; private set; }

		[Reactive] public bool IsMovieActive { get; private set; }
		[Reactive] public bool IsVideoRecording { get; private set; }
		[Reactive] public bool IsSoundRecording { get; private set; }

		[Reactive] public bool IsNetPlayActive { get; private set; }
		[Reactive] public bool IsNetPlayClient { get; private set; }
		[Reactive] public bool IsNetPlayServer { get; private set; }

		[Reactive] public ObservableCollection<RecentItem> RecentItems { get; private set; }
		[Reactive] public bool HasRecentItems { get; private set; }
		public ReactiveCommand<RecentItem, Unit> OpenRecentCommand { get; }

		//For designer
		public MainMenuViewModel() : this(new MainWindowViewModel()) { }

		public MainMenuViewModel(MainWindowViewModel windowModel)
		{
			MainWindow = windowModel;

			OpenRecentCommand = ReactiveCommand.Create<RecentItem>(OpenRecent);

			RecentItems = ConfigManager.Config.RecentFiles.Items;
			this.WhenAnyValue(x => x.RecentItems.Count).Subscribe(count => {
				HasRecentItems = count > 0;
			});

			this.WhenAnyValue(x => x.MainWindow.RomInfo).Subscribe(x => {
				IsGameRunning = x.Format != RomFormat.Unknown;

				IsFdsGame = x.Format == RomFormat.Fds;
				IsVsSystemGame = x.Format == RomFormat.VsSystem || x.Format == RomFormat.VsDualSystem;
				IsVsDualSystemGame = x.Format == RomFormat.VsDualSystem;
				IsNesGame = x.ConsoleType == ConsoleType.Nes;
				IsGbGame = x.ConsoleType == ConsoleType.GameboyColor || x.ConsoleType == ConsoleType.Gameboy;
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

		internal void UpdateToolSubMenus()
		{
			IsVideoRecording = RecordApi.AviIsRecording();
			IsSoundRecording = RecordApi.WaveIsRecording();
			IsMovieActive = RecordApi.MovieRecording() || RecordApi.MoviePlaying();
			
			IsNetPlayClient = NetplayApi.IsConnected();
			IsNetPlayServer = NetplayApi.IsServerRunning();
			IsNetPlayActive = IsNetPlayClient || IsNetPlayServer;
		}
	}
}
