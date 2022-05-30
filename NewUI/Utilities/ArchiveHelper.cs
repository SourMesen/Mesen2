using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Utilities
{
	public class ArchiveHelper
	{
		public unsafe static List<ArchiveRomEntry> GetArchiveRomList(string archivePath)
		{
			//Split the array on the [!|!] delimiter
			byte[] buffer = new byte[100000];
			fixed(byte* ptr = buffer) {
				EmuApi.GetArchiveRomList(archivePath, (IntPtr)ptr, 100000);
			}

			List<List<byte>> filenames = new List<List<byte>>();
			List<byte> filenameBytes = new List<byte>();
			for(int i = 0; i < buffer.Length - 5; i++) {
				if(buffer[i] == 0) {
					break;
				}

				if(buffer[i] == '[' && buffer[i + 1] == '!' && buffer[i + 2] == '|' && buffer[i + 3] == '!' && buffer[i + 4] == ']') {
					if(filenameBytes.Count > 0) {
						filenames.Add(filenameBytes);
					}
					filenameBytes = new List<byte>();
					i += 4;
				} else {
					filenameBytes.Add(buffer[i]);
				}
			}
			if(filenameBytes.Count > 0) {
				filenames.Add(filenameBytes);
			}

			List<ArchiveRomEntry> entries = new List<ArchiveRomEntry>();

			//Check whether or not each string is a valid utf8 filename, if not decode it using the system's default encoding.
			//This is necessary because zip files do not have any rules when it comes to encoding filenames
			for(int i = 0; i < filenames.Count; i++) {
				byte[] originalBytes = filenames[i].ToArray();
				string utf8Filename = Encoding.UTF8.GetString(originalBytes);
				byte[] convertedBytes = Encoding.UTF8.GetBytes(utf8Filename);
				bool equal = true;
				if(originalBytes.Length == convertedBytes.Length) {
					for(int j = 0; j < convertedBytes.Length; j++) {
						if(convertedBytes[j] != originalBytes[j]) {
							equal = false;
							break;
						}
					}
				} else {
					equal = false;
				}

				if(!equal) {
					//String doesn't appear to be an utf8 string, use the system's default encoding
					entries.Add(new ArchiveRomEntry() { Filename = Encoding.Default.GetString(originalBytes), IsUtf8 = false });
				} else {
					entries.Add(new ArchiveRomEntry() { Filename = utf8Filename, IsUtf8 = true });
				}
			}

			return entries;
		}
	}

	public class ArchiveRomEntry
	{
		public string Filename = "";
		public bool IsUtf8;

		public override string ToString()
		{
			return Filename;
		}
	}
}
