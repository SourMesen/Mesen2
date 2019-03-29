using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;
using Mesen.GUI.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Emulation
{
	public static class SaveStateManager
	{
		private const int NumberOfSaveSlots = 10;

		public static void UpdateStateMenu(ToolStripMenuItem menu, bool forSave)
		{
			for(uint i = 1; i <= NumberOfSaveSlots + (forSave ? 0 : 1); i++) {
				Int64 fileTime = EmuApi.GetStateInfo(i);
				string label;
				bool isAutoSaveSlot = i == NumberOfSaveSlots + 1;
				string slotName = isAutoSaveSlot ? "Auto" : i.ToString();

				if(fileTime == 0) {
					label = slotName + ". " + ResourceHelper.GetMessage("EmptyState");
				} else {
					DateTime dateTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(fileTime).ToLocalTime();
					label = slotName + ". " + dateTime.ToShortDateString() + " " + dateTime.ToShortTimeString();
				}

				if(i == NumberOfSaveSlots + 1) {
					//Autosave slot (load only)
					menu.DropDownItems[NumberOfSaveSlots + 1].Text = label;
				} else {
					menu.DropDownItems[(int)i - 1].Text = label;
				}
			}
		}

		public static void InitializeStateMenu(ToolStripMenuItem menu, bool forSave, ShortcutHandler shortcutHandler)
		{
			Action<uint> addSaveStateInfo = (i) => {
				ToolStripMenuItem item = new ToolStripMenuItem();
				menu.DropDownItems.Add(item);

				if(forSave) {
					shortcutHandler.BindShortcut(item, (EmulatorShortcut)((int)EmulatorShortcut.SaveStateSlot1 + i - 1));
				} else {
					shortcutHandler.BindShortcut(item, (EmulatorShortcut)((int)EmulatorShortcut.LoadStateSlot1 + i - 1));
				}
			};

			for(uint i = 1; i <= NumberOfSaveSlots; i++) {
				addSaveStateInfo(i);
			}

			if(!forSave) {
				menu.DropDownItems.Add("-");
				addSaveStateInfo(NumberOfSaveSlots + 1);
				menu.DropDownItems.Add("-");
				ToolStripMenuItem loadFromFile = new ToolStripMenuItem(ResourceHelper.GetMessage("LoadFromFile"), Resources.Folder);
				menu.DropDownItems.Add(loadFromFile);
				shortcutHandler.BindShortcut(loadFromFile, EmulatorShortcut.LoadStateFromFile);
			} else {
				menu.DropDownItems.Add("-");
				ToolStripMenuItem saveToFile = new ToolStripMenuItem(ResourceHelper.GetMessage("SaveToFile"), Resources.SaveFloppy);
				menu.DropDownItems.Add(saveToFile);
				shortcutHandler.BindShortcut(saveToFile, EmulatorShortcut.SaveStateToFile);
			}
		}

		public static void SaveState(uint slot)
		{
			if(EmuRunner.IsRunning()) {
				EmuApi.SaveState(slot);
			}
		}

		public static void LoadState(uint slot)
		{
			if(EmuRunner.IsRunning()) {
				EmuApi.LoadState(slot);
			}
		}

		public static void LoadStateFromFile()
		{
			if(EmuRunner.IsRunning()) {
				using(OpenFileDialog ofd = new OpenFileDialog()) {
					ofd.InitialDirectory = ConfigManager.SaveStateFolder;
					ofd.SetFilter(ResourceHelper.GetMessage("FilterSavestate"));
					if(ofd.ShowDialog(Application.OpenForms[0]) == DialogResult.OK) {
						EmuApi.LoadStateFile(ofd.FileName);
					}
				}
			}
		}

		public static void SaveStateToFile()
		{
			if(EmuRunner.IsRunning()) {
				using(SaveFileDialog sfd = new SaveFileDialog()) {
					sfd.InitialDirectory = ConfigManager.SaveStateFolder;
					sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".mst";
					sfd.SetFilter(ResourceHelper.GetMessage("FilterSavestate"));
					if(sfd.ShowDialog(Application.OpenForms[0]) == DialogResult.OK) {
						EmuApi.SaveStateFile(sfd.FileName);
					}
				}
			}
		}
	}
}
