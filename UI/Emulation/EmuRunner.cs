using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Emulation
{
	public static class EmuRunner
	{
		private static Thread _emuThread = null;

		public static void LoadRom(ResourcePath romPath, ResourcePath? patchPath = null)
		{
			if(!frmSelectRom.SelectRom(ref romPath)) {
				return;
			}

			EmuApi.LoadRom(romPath, patchPath);
			ConfigManager.Config.RecentFiles.AddRecentFile(romPath, patchPath);
			StartEmulation();
		}

		public static void LoadRecentGame(string recentGameArchivePath)
		{
			EmuApi.LoadRecentGame(recentGameArchivePath, false /* TODO , ConfigManager.Config.Preferences.GameSelectionScreenResetGame */);
			StartEmulation();
		}

		private static void StartEmulation()
		{
			_emuThread = new Thread(() => {
				EmuApi.Run();
				_emuThread = null;
			});
			_emuThread.Start();
		}

		public static bool IsRunning()
		{
			return _emuThread != null;
		}
	}
}
