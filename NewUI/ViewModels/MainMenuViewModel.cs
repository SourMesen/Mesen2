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

		[Reactive] public ObservableCollection<RecentItem> RecentItems { get; private set; }
		[Reactive] public bool HasRecentItems { get; private set; }
		public ReactiveCommand<RecentItem, Unit> OpenRecentCommand { get; }

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
				IsNesGame = x.Format == RomFormat.iNes || x.Format == RomFormat.Fds || x.Format == RomFormat.Unif;
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
