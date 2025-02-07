using Mesen.Config;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class FolderHelper
	{
		private static HashSet<string> _romExtensions = new HashSet<string>() {
			".sfc", ".smc", ".fig", ".swc", ".bs", ".st",
			".gb", ".gbc", ".gbx",
			".nes", ".unif", ".unf", ".fds", ".qd", ".studybox",
			".pce", ".sgx", ".cue",
			".sms", ".gg", ".sg", ".col",
			".gba",
			".ws", ".wsc"
		};

		public static bool IsRomFile(string path)
		{
			string ext = Path.GetExtension(path).ToLower();
			return _romExtensions.Contains(ext);
		}

		public static bool IsArchiveFile(string path)
		{
			string ext = Path.GetExtension(path).ToLower();
			return ext == ".7z" || ext == ".zip";
		}

		public static bool CheckFolderPermissions(string folder, bool checkWritePermission = true)
		{
			if(!Directory.Exists(folder)) {
				try {
					if(string.IsNullOrWhiteSpace(folder)) {
						return false;
					}
					Directory.CreateDirectory(folder);
				} catch {
					return false;
				}
			}
			if(checkWritePermission) {
				try {
					string fileName = Guid.NewGuid().ToString() + ".txt";
					File.WriteAllText(Path.Combine(folder, fileName), "");
					File.Delete(Path.Combine(folder, fileName));
				} catch {
					return false;
				}
			}
			return true;
		}
	}
}
