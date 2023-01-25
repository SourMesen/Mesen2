using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Utilities
{
	public class TblByteCharConverter
	{
		private Dictionary<TblKey, string> _tblRules;
		private Dictionary<string, TblKey> _reverseTblRules;
		private List<string> _stringList;
		private bool[] _hasPotentialRule = new bool[256];

		public TblByteCharConverter(Dictionary<TblKey, string> tblRules)
		{
			this._tblRules = tblRules;
			foreach(KeyValuePair<TblKey, string> kvp in tblRules) {
				_hasPotentialRule[kvp.Key.Key & 0xFF] = true;
			}

			this._stringList = new List<string>();
			this._reverseTblRules = new Dictionary<string, TblKey>();
			foreach(KeyValuePair<TblKey, string> kvp in tblRules) {
				this._stringList.Add(kvp.Value);
				this._reverseTblRules[kvp.Value] = kvp.Key;
			}
			this._stringList.Sort(new Comparison<string>((a, b) => {
				if(a.Length > b.Length) {
					return 1;
				} else if(a.Length < b.Length) {
					return -1;
				} else {
					return string.Compare(a, b);
				}
			}));
		}

		public string ToChar(UInt64 value, out int keyLength)
		{
			if(!_hasPotentialRule[value & 0xFF]) {
				keyLength = 1;
				return ".";
			}

			int byteCount = 8;
			string? result;
			while(!_tblRules.TryGetValue(new TblKey() { Key = value, Length = byteCount }, out result) && byteCount > 0) {
				value &= ~(((UInt64)0xFF) << (8 * (byteCount - 1)));
				byteCount--;
			}

			if(result != null) {
				keyLength = byteCount;
				return result;
			} else {
				keyLength = 1;
				return ".";
			}
		}

		public byte[] GetBytes(string text)
		{
			List<byte> bytes = new List<byte>();

			bool match = false;
			while(text.Length > 0) {
				do {
					match = false;
					foreach(string key in _stringList) {
						//Without an ordinal comparison, it's possible for an empty string to "StartsWith" a non-empty string (e.g replacement char U+FFFD)
						//This causes a crash here (because key.Length > text.Length)
						if(text.StartsWith(key, StringComparison.Ordinal)) {
							bytes.AddRange(_reverseTblRules[key].GetBytes());
							text = text.Substring(key.Length);
							match = true;
							break;
						}
					}
				} while(match);

				if(!match && text.Length > 0) {
					bytes.Add(0);
					text = text.Substring(1);
				}
			}

			return bytes.ToArray();
		}
	}
}
