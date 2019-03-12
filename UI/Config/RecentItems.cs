using Mesen.GUI.Emulation;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Config
{
	public class RecentItems
	{
		private const int MaxRecentFiles = 10;
		public List<RecentItem> Items;

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
			ConfigManager.ApplyChanges();
		}

		public List<ToolStripItem> GetMenuItems()
		{
			List<ToolStripItem> menuItems = new List<ToolStripItem>();
			foreach(RecentItem recentItem in Items) {
				ToolStripMenuItem tsmi = new ToolStripMenuItem();
				tsmi.Text = recentItem.ToString();
				tsmi.Click += (object sender, EventArgs args) => {
					EmuRunner.LoadRom(recentItem.RomFile, recentItem.PatchFile);
				};
				menuItems.Add(tsmi);
			}
			return menuItems;
		}
	}

	public class RecentItem
	{
		public ResourcePath RomFile;
		public ResourcePath? PatchFile;

		public override string ToString()
		{
			string path = RomFile.ReadablePath.Replace("&", "&&");
			string file = Path.GetFileName(path);
			string folder = Path.GetDirectoryName(path);

			string text = file;
			if(PatchFile.HasValue) {
				text += " [" + Path.GetFileName(PatchFile.Value) + "]";
			}
			text += " (" + folder + ")";
			return text;
		}
	}
}
