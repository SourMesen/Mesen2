using System;
using System.Collections.Generic;
using System.IO.Compression;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities;

class DependencyHelper
{
	public static void ExtractNativeDependencies(string dest)
	{
		using(Stream? depStream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Dependencies.zip")) {
			if(depStream == null) {
				throw new Exception("Missing dependencies.zip");
			}

			using ZipArchive zip = new(depStream);
			foreach(ZipArchiveEntry entry in zip.Entries) {
				try {
					if(entry.FullName.StartsWith("Internal")) {
						continue;
					}

					string path = Path.Combine(dest, entry.FullName);
					entry.ExternalAttributes = 0;
					if(File.Exists(path)) {
						if(Path.GetExtension(path)?.ToLower() == ".bin") {
							//Don't overwrite BS-X bin files if they already exist on the disk
							continue;
						}

						FileInfo fileInfo = new(path);
						if(fileInfo.LastWriteTime != entry.LastWriteTime || fileInfo.Length != entry.Length) {
							entry.ExtractToFile(path, true);
						}
					} else {
						string? folderName = Path.GetDirectoryName(path);
						if(folderName != null && !Directory.Exists(folderName)) {
							//Create any missing directory (e.g Satellaview)
							Directory.CreateDirectory(folderName);
						}
						entry.ExtractToFile(path, true);
					}
				} catch {

				}
			}
		}
	}

	public static string? GetFileContent(string filename)
	{
		using Stream? depStream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Dependencies.zip");
		if(depStream != null) {
			using ZipArchive zip = new(depStream);
			foreach(ZipArchiveEntry entry in zip.Entries) {
				if(entry.Name == filename) {
					using Stream entryStream = entry.Open();
					using StreamReader reader = new StreamReader(entryStream);
					return reader.ReadToEnd();
				}
			}
		}
		return null;
	}
}
