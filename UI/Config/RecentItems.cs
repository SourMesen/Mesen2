using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class RecentItems
	{
		private const int MaxRecentFiles = 10;
		public List<RecentItem> Items { get; set; } = new List<RecentItem>();

		public void AddRecentFile(ResourcePath romFile, ResourcePath? patchFile)
		{
			if(patchFile.HasValue && string.IsNullOrWhiteSpace(patchFile)) {
				patchFile = null;
			}

			RecentItem? existingItem = Items.Where((item) => item.RomFile == romFile && item.PatchFile == patchFile).FirstOrDefault();
			if(existingItem != null) {
				Items.Remove(existingItem);
			}
			RecentItem recentItem = new RecentItem { RomFile = romFile, PatchFile = patchFile };

			Items.Insert(0, recentItem);
			if(Items.Count > RecentItems.MaxRecentFiles) {
				Items.RemoveAt(RecentItems.MaxRecentFiles);
			}
		}
	}

	public class RecentItem
	{
		public ResourcePath RomFile { get; set; }
		public ResourcePath? PatchFile { get; set; }

		public string DisplayText
		{
			get
			{
				string text = RomFile.ReadablePath;
				if(PatchFile.HasValue) {
					text += " [" + Path.GetFileName(PatchFile.Value) + "]";
				}
				return text;
			}
		}

		public string ShortenedFolder
		{
			get
			{
				string[] folderParts = RomFile.Folder.Split(new char[2] { '\\', '/' });
				string folder;
				if(folderParts.Length > 4) {
					folder = folderParts[0] + Path.DirectorySeparatorChar + folderParts[1] + Path.DirectorySeparatorChar + folderParts[2] + Path.DirectorySeparatorChar + ".." + Path.DirectorySeparatorChar + folderParts[folderParts.Length - 1];
				} else {
					folder = RomFile.Folder;
				}
				return $"({folder})";
			}
		}
	}
}
