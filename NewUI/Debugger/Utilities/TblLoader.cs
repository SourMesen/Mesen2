using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public class TblLoader
	{
		public static TblByteCharConverter? Load(string[] fileContents)
		{
			try {
				Dictionary<TblKey, string> dict = new Dictionary<TblKey, string>();

				for(int i = 0; i < fileContents.Length; i++) {
					if(!string.IsNullOrWhiteSpace(fileContents[i])) {
						string[] data = fileContents[i].Split('=');
						if(data.Length == 2) {
							data[0] = data[0].Replace(" ", "");
							if(data[0].Length % 2 == 0 && Regex.IsMatch(data[0], "[0-9A-Fa-f]+")) {
								TblKey key = new TblKey();

								for(int j = 0; j < data[0].Length; j+=2) {
									byte result = byte.Parse(data[0].Substring(j, 2), System.Globalization.NumberStyles.HexNumber);
									key.Key |= (UInt64)result << (8 * j / 2);
									key.Length++;
								}

								dict[key] = string.IsNullOrEmpty(data[1]) ? " " : data[1];
							}
						}
					}
				}

				return new TblByteCharConverter(dict);
			} catch {
				return null;
			}
		}
	}

	public struct TblKey
	{
		public UInt64 Key;
		public int Length;

		public byte[] GetBytes()
		{
			byte[] bytes = new byte[this.Length];
			UInt64 value = this.Key;
			for(int i = 0; i < this.Length; i++) {
				bytes[i] = (byte)value;
				value >>= 8;
			}
			return bytes;
		}
	}
}
