using Mesen.GUI.Config;
using Mesen.GUI.Emulation;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Utilities
{
	public static class RandomGameHelper
	{
		public static void LoadRandomGame()
		{
			IEnumerable<string> gameFolders;
			SearchOption searchOptions = SearchOption.TopDirectoryOnly;
			if(ConfigManager.Config.Preferences.OverrideGameFolder && Directory.Exists(ConfigManager.Config.Preferences.GameFolder)) {
				gameFolders = new List<string>() { ConfigManager.Config.Preferences.GameFolder };
				searchOptions = SearchOption.AllDirectories;
			} else {
				gameFolders = ConfigManager.Config.RecentFiles.Items.Select(recentFile => recentFile.RomFile.Folder).Distinct();
			}

			List<string> gameRoms = new List<string>();
			foreach(string folder in gameFolders) {
				if(Directory.Exists(folder)) {
					foreach(string file in Directory.EnumerateFiles(folder, "*.*", searchOptions)) {
						if(FolderHelper.IsRomFile(file) || (searchOptions == SearchOption.AllDirectories && FolderHelper.IsArchiveFile(file))) {
							gameRoms.Add(file);
						}
					}
				}
			}

			if(gameRoms.Count == 0) {
				MesenMsgBox.Show("RandomGameNoGameFound", MessageBoxButtons.OK, MessageBoxIcon.Information);
			} else {
				int retryCount = 0;
				Random random = new Random();
				do {
					string randomGame = gameRoms[random.Next(gameRoms.Count)];
					if(FolderHelper.IsArchiveFile(randomGame)) {
						List<ArchiveRomEntry> archiveRomList = ArchiveHelper.GetArchiveRomList(randomGame);
						if(archiveRomList.Count > 0) {
							ResourcePath res = new ResourcePath() {
								InnerFile = archiveRomList[0].Filename,
								Path = randomGame
							};
							if(!archiveRomList[0].IsUtf8) {
								res.InnerFileIndex = 1;
							}
							EmuRunner.LoadRom(res);
							break;
						} else {
							retryCount++;
						}
					} else {
						EmuRunner.LoadRom(randomGame);
						break;
					}
				} while(retryCount < 5);
			}
		}
	}
}
