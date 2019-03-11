using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

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

		public static bool IsRunning()
		{
			return _emuThread != null;
		}
		
		public static void SaveState(uint slot)
		{
			if(_emuThread != null) {
				//EmuApi.SaveState(slot);
			}
		}

		public static void LoadState(uint slot)
		{
			if(_emuThread != null) {
				//EmuApi.LoadState(slot);
			}
		}

		public static void LoadStateFromFile()
		{
			if(_emuThread != null) {
				using(OpenFileDialog ofd = new OpenFileDialog()) {
					ofd.InitialDirectory = ConfigManager.SaveStateFolder;
					ofd.SetFilter(ResourceHelper.GetMessage("FilterSavestate"));
					if(ofd.ShowDialog(Application.OpenForms[0]) == DialogResult.OK) {
						//EmuApi.LoadStateFile(ofd.FileName);
					}
				}
			}
		}

		public static void SaveStateToFile()
		{
			if(_emuThread != null) {
				using(SaveFileDialog sfd = new SaveFileDialog()) {
					sfd.InitialDirectory = ConfigManager.SaveStateFolder;
					//TODO
					//sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".mst";
					sfd.SetFilter(ResourceHelper.GetMessage("FilterSavestate"));
					if(sfd.ShowDialog(Application.OpenForms[0]) == DialogResult.OK) {
						//EmuApi.SaveStateFile(sfd.FileName);
					}
				}
			}
		}
	}
}
