using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;
using Mesen.GUI.Properties;
using System;
using System.Collections.Generic;
using System.IO;
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
				string statePath = Path.Combine(ConfigManager.SaveStateFolder, EmuApi.GetRomInfo().GetRomName() + "_" + i + ".mss");
				string label;
				bool isAutoSaveSlot = i == NumberOfSaveSlots + 1;
				string slotName = isAutoSaveSlot ? "Auto" : i.ToString();

				if(!File.Exists(statePath)) {
					label = slotName + ". " + ResourceHelper.GetMessage("EmptyState");
				} else {
					DateTime dateTime = new FileInfo(statePath).LastWriteTime;
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

				ToolStripMenuItem loadDialog = new ToolStripMenuItem(ResourceHelper.GetMessage("LoadStateDialog"), Resources.SplitView);
				menu.DropDownItems.Add(loadDialog);
				shortcutHandler.BindShortcut(loadDialog, EmulatorShortcut.LoadStateDialog, () => EmuRunner.IsRunning() && !NetplayApi.IsConnected());

				ToolStripMenuItem loadFromFile = new ToolStripMenuItem(ResourceHelper.GetMessage("LoadFromFile"), Resources.Folder);
				menu.DropDownItems.Add(loadFromFile);
				shortcutHandler.BindShortcut(loadFromFile, EmulatorShortcut.LoadStateFromFile);
			} else {
				menu.DropDownItems.Add("-");
				ToolStripMenuItem saveDialog = new ToolStripMenuItem(ResourceHelper.GetMessage("SaveStateDialog"), Resources.SplitView);
				menu.DropDownItems.Add(saveDialog);
				shortcutHandler.BindShortcut(saveDialog, EmulatorShortcut.SaveStateDialog, () => EmuRunner.IsRunning() && !NetplayApi.IsConnected());

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
					if(ofd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
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
					sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".mss";
					sfd.SetFilter(ResourceHelper.GetMessage("FilterSavestate"));
					if(sfd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
						EmuApi.SaveStateFile(sfd.FileName);
					}
				}
			}
		}
	}
}
