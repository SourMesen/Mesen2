using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Mesen.GUI
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

			_emuThread = new Thread(() => {
				EmuApi.Run();
			});
			_emuThread.Start();
		}
	}
}
