using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class RecentGamesViewModel : ViewModelBase
	{
		[Reactive] public bool Visible { get; set; }
		[Reactive] public bool NeedResume { get; private set; }
		[Reactive] public string Title { get; private set; } = "";
		[Reactive] public GameScreenMode Mode { get; private set; }
		[Reactive] public List<RecentGameInfo> GameEntries { get; private set; } = new List<RecentGameInfo>();
		
		public RecentGamesViewModel()
		{
			Visible = ConfigManager.Config.Preferences.GameSelectionScreenMode != GameSelectionMode.Disabled;
		}

		public void Init(GameScreenMode mode)
		{
			if(mode == GameScreenMode.RecentGames && ConfigManager.Config.Preferences.GameSelectionScreenMode == GameSelectionMode.Disabled) {
				Visible = false;
				GameEntries = new List<RecentGameInfo>();
				return;
			} else if(mode != GameScreenMode.RecentGames && Mode == mode && Visible) {
				Visible = false;
				if(NeedResume) {
					EmuApi.Resume();
				}
				return;
			}

			if(Mode == mode && Visible && GameEntries.Count > 0) {
				//Prevent flickering when closing the config window while no game is running
				//No need to update anything if the game selection screen is already visible
				return;
			}

			Mode = mode;

			List<RecentGameInfo> entries = new();

			if(mode == GameScreenMode.RecentGames) {
				NeedResume = false;
				Title = string.Empty;

				List<string> files = Directory.GetFiles(ConfigManager.RecentGamesFolder, "*.rgd").OrderByDescending((file) => new FileInfo(file).LastWriteTime).ToList();
				for(int i = 0; i < files.Count && entries.Count < 72; i++) {
					entries.Add(new RecentGameInfo() { FileName = files[i], Name = Path.GetFileNameWithoutExtension(files[i]) });
				}
			} else {
				if(!Visible) {
					NeedResume = Pause();
				}

				Title = mode == GameScreenMode.LoadState ? ResourceHelper.GetMessage("LoadStateDialog") : ResourceHelper.GetMessage("SaveStateDialog");
				
				string romName = EmuApi.GetRomInfo().GetRomName();
				for(int i = 0; i < (mode == GameScreenMode.LoadState ? 11 : 10); i++) {
					entries.Add(new RecentGameInfo() {
						FileName = Path.Combine(ConfigManager.SaveStateFolder, romName + "_" + (i + 1) + "." + FileDialogHelper.MesenSaveStateExt),
						StateIndex = i + 1,
						Name = i == 10 ? ResourceHelper.GetMessage("AutoSave") : ResourceHelper.GetMessage("SlotNumber", i + 1),
						SaveMode = mode == GameScreenMode.SaveState
					});
				}
				if(mode == GameScreenMode.LoadState) {
					entries.Add(new RecentGameInfo() {
						FileName = Path.Combine(ConfigManager.RecentGamesFolder, romName + ".rgd"),
						Name = ResourceHelper.GetMessage("LastSession")
					});
				}
			}

			Visible = entries.Count > 0;
			GameEntries = entries;
		}

		private bool Pause()
		{
			if(!EmuApi.IsPaused()) {
				EmuApi.Pause();
				return true;
			}
			return false;
		}
	}

	public enum GameScreenMode
	{
		RecentGames,
		LoadState,
		SaveState
	}

	public class RecentGameInfo
	{
		public string FileName { get; set; } = "";
		public int StateIndex { get; set; } = -1;
		public string Name { get; set; } = "";
		public bool SaveMode { get; set; } = false;

		public bool IsEnabled()
		{
			return SaveMode || File.Exists(FileName);
		}

		public void Load()
		{
			if(StateIndex > 0) {
				Task.Run(() => {
					//Run in another thread to prevent deadlocks etc. when emulator notifications are processed UI-side
					if(SaveMode) {
						EmuApi.SaveState((uint)StateIndex);
					} else {
						EmuApi.LoadState((uint)StateIndex);
					}
					EmuApi.Resume();
				});
			} else {
				LoadRomHelper.LoadRecentGame(FileName, false);
				EmuApi.Resume();
			}
		}
	}
}
