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
			".sfc", ".smc", ".fig", ".swc", ".bs",
			".gb", ".gbc",
			".nes", ".unif", ".unf", ".fds", ".studybox",
			".pce", ".sgx", ".cue",
			".sms", ".gg", ".sg",
			".gba",
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
	csharp
try 
{
    string fileName = Guid.NewGuid().ToString() + ".txt";
    string sanitizedFolder = Path.GetFullPath(folder); // Sanitize the folder path
    
    // Validate directory exists before writing the file
    if (!Directory.Exists(sanitizedFolder))
    {
        Directory.CreateDirectory(sanitizedFolder);
    }
    
    File.WriteAllText(Path.Combine(sanitizedFolder, fileName), "");
    
    // Delete the file after writing
    File.Delete(Path.Combine(sanitizedFolder, fileName));
} 
catch (Exception ex) 
{
    // Handle or log the exception
    Console.WriteLine("An error occurred: " + ex.Message);
}

					return false;
				}
			}
			return true;
		}
	}
}
