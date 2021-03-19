using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class RecentItems
	{
		private const int MaxRecentFiles = 10;
		public List<RecentItem> Items = new List<RecentItem>();

		public void AddRecentFile(ResourcePath romFile, ResourcePath? patchFile)
		{
			if(patchFile.HasValue && string.IsNullOrWhiteSpace(patchFile)) {
				patchFile = null;
			}

			RecentItem existingItem = Items.Where((item) => item.RomFile == romFile && item.PatchFile == patchFile).FirstOrDefault();
			if(existingItem != null) {
				Items.Remove(existingItem);
			}
			RecentItem recentItem = new RecentItem { RomFile = romFile, PatchFile = patchFile };

			Items.Insert(0, recentItem);
			if(Items.Count > RecentItems.MaxRecentFiles) {
				Items.RemoveAt(RecentItems.MaxRecentFiles);
			}
			ConfigManager.SaveConfig();
		}

		/*public List<ToolStripItem> GetMenuItems()
		{
			List<ToolStripItem> menuItems = new List<ToolStripItem>();
			foreach(RecentItem recentItem in Items) {
				ToolStripMenuItem tsmi = new ToolStripMenuItem();
				tsmi.Text = recentItem.ToString();
				tsmi.Click += (object sender, EventArgs args) => {
					EmuRunner.LoadRom(recentItem.RomFile, recentItem.PatchFile);
				};

				//Display shortened folder path as the "shortcut"				
				tsmi.ShortcutKeyDisplayString = "(" + recentItem.GetShortenedFolder() + ")";
				
				menuItems.Add(tsmi);
			}

			menuItems.Add(new ToolStripSeparator());

			ToolStripMenuItem clearHistory = new ToolStripMenuItem();
			clearHistory.Text = ResourceHelper.GetMessage("ClearHistory");
			clearHistory.Image = Resources.Close;
			clearHistory.Click += (object sender, EventArgs args) => {
				ConfigManager.Config.RecentFiles.Items.Clear();
			};
			menuItems.Add(clearHistory);
			
			return menuItems;
		}*/
	}

	public class RecentItem
	{
		public ResourcePath RomFile;
		public ResourcePath? PatchFile;

		public override string ToString()
		{
			string path = RomFile.ReadablePath.Replace("&", "&&");
			string text = Path.GetFileName(path);
			if(PatchFile.HasValue) {
				text += " [" + Path.GetFileName(PatchFile.Value) + "]";
			}
			return text;
		}

		public string GetShortenedFolder()
		{
			string[] folderParts = RomFile.Folder.Split(new char[2] { '\\', '/' });
			if(folderParts.Length > 4) {
				return folderParts[0] + Path.DirectorySeparatorChar + folderParts[1] + Path.DirectorySeparatorChar + folderParts[2] + Path.DirectorySeparatorChar + ".." + Path.DirectorySeparatorChar + folderParts[folderParts.Length - 1];
			} else {
				return RomFile.Folder;
			}
		}
	}
}
